/******************************************************** 
* author: scofieldzhu
* time:2025/11/24
*******************************************************/
#include "dicomReaderDlg.h"
#include <QFileInfo>
#include <QMessageBox>
#include <vtkDICOMReader.h>
#include <vtkStringArray.h>
#include <vtkMatrix3x3.h>
#include <vtkDICOMItem.h>
#include <vtkIntArray.h>
#include <vtkDICOMMetaData.h>
#include <QDebug>
#include <vtkImageReslice.h>
#include <vtkImageChangeInformation.h>
#include <vtkMatrix4x4.h>
#include <vtkImageFlip.h>
#include "dicomReader.h"

namespace {
    const vtkDICOMTag kPixelDataTag(0x7fe0, 0x0010);
    const vtkDICOMTag kCharsetTag(0x0008, 0x0005);
    const vtkDICOMTag kSeriesUIDTag(0x0020, 0x000E);
    const vtkDICOMTag kSpacingBetweenSlicesTag(0x0018, 0x0088);

    const vtkDICOMTag kImagePositionTag(0x0020, 0x0032);
    const vtkDICOMTag kImageOrientationTag(0x0020, 0x0037);
    const vtkDICOMTag kFrameOfReferenceUIDTag(0x0020, 0x0052);
    const vtkDICOMTag kGantryDetectorTag(0x0018, 0x1120);

    const vtkDICOMTag kPatientNameTag(0x0010, 0x0010);
    const vtkDICOMTag kPatientAgeTag(0x0010, 0x1010);
    const vtkDICOMTag kPatientSexTag(0x0010, 0x0040);
    const vtkDICOMTag kAcquisitionDateTag(0x0008, 0x0022);
    const vtkDICOMTag kAcquisitionTimeTag(0x0008, 0x0032);
    const vtkDICOMTag kSliceThicknessTag(0x0018, 0x0050);

    const vtkDICOMTag kManufacturerTag(0x0008, 0x0070);

    const vtkDICOMTag kWindowCenterTag(0x0028, 0x1050);
    const vtkDICOMTag kWindowWidthTag(0x0028, 0x1051);

    void ConvertLPSToRAS_Physical(vtkImageData* input)
    {
        // 1. X 轴翻转
        vtkNew<vtkImageFlip> flipX;
        flipX->SetInputData(input);
        flipX->SetFilteredAxis(0); // 翻转 X
        flipX->Update();

        // 2. Y 轴翻转
        vtkNew<vtkImageFlip> flipY;
        flipY->SetInputConnection(flipX->GetOutputPort());
        flipY->SetFilteredAxis(1); // 翻转 Y
        flipY->Update();

        // 3. 修正 Origin（可选，视具体需求）
        // 翻转后，物理原点通常需要根据图像 bounds 重新计算
        // 如果你只关心像素排列，不关心物理坐标，到这里就可以了
        input->DeepCopy(flipY->GetOutput());
    }

    void CenterImage(vtkImageData* image)
    {
        vtkNew<vtkImageChangeInformation> changer;
        changer->SetInputData(image);
        changer->CenterImageOn();
        changer->Update();
        image->DeepCopy(changer->GetOutput());
    }

    void AdjustImageOrientation(vtkImageData* imgdata, vtkDICOMReader* dicomReader)
    {
        vtkMatrix4x4* P = dicomReader->GetPatientMatrix();
        double rowVec[3]; // 通常指向 Left
        double colVec[3]; // 通常指向 Posterior
        double sliceVec[3]; // 通常指向 Superior

        for (int i = 0; i < 3; ++i) {
            rowVec[i] = P->GetElement(i, 0);
            colVec[i] = P->GetElement(i, 1);
            sliceVec[i] = P->GetElement(i, 2);
        }

        vtkNew<vtkMatrix4x4> invP;
        //invP->DeepCopy(P);
        vtkMatrix4x4::Invert(P, invP);  // invP = P^-1

        // 2. 构造 RAS -> LPS 的矩阵（注意：和 LPS->RAS 相同）
        vtkNew<vtkMatrix4x4> rasToLps;
        rasToLps->Identity();
        //rasToLps->SetElement(0, 0, -1.0); // R -> L
        //rasToLps->SetElement(1, 1, -1.0); // A -> P
        // Z 轴不变

        // 3. A = P^-1 * T_RAS→LPS
        vtkNew<vtkMatrix4x4> A;
        vtkMatrix4x4::Multiply4x4(invP, rasToLps, A);

        // 2. 用 Reslice 重采样到“方向=单位矩阵”的新图像
        vtkNew<vtkImageReslice> reslice;
        reslice->SetInputData(imgdata);
        reslice->SetResliceAxes(A);
        reslice->SetInterpolationModeToLinear();

        // 输出 spacing / extent 通常可以直接沿用原图
        double spacing[3];
        imgdata->GetSpacing(spacing);
        reslice->SetOutputSpacing(spacing);

        int extent[6];
        imgdata->GetExtent(extent);
        reslice->SetOutputExtent(extent);
        reslice->Update();

        imgdata->DeepCopy(reslice->GetOutput());
    }
}

DICOMReaderDlg::DICOMReaderDlg(QWidget* p, DICOMReader* pd)
	:QDialog(p, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    pd_(pd),
    file_finder_(vtkSmartPointer<vtkDICOMDirectory>::New())
{
	ui_.setupUi(this);
    setWindowTitle(tr("Reading dicom data dialog"));

    ui_.patientTable->setColumnCount(6);
    QStringList headers;
    headers << tr("Patient Name") 
            << tr("Patient ID")
            << tr("Birth Date") 
            << tr("Sex")
            << tr("Studies")
            << tr("Last Study Date")
            ;
    ui_.patientTable->setHorizontalHeaderLabels(headers);
    ui_.patientTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_.patientTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_.patientTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui_.patientTable->verticalHeader()->setVisible(false);    

    ui_.studyTable->setColumnCount(4);
    headers.clear();
    headers << tr("Study Date") 
            << tr("Study ID")
            << tr("Study Description") 
            << tr("Series")
            ;
    ui_.studyTable->setHorizontalHeaderLabels(headers);
    ui_.studyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_.studyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_.studyTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui_.studyTable->verticalHeader()->setVisible(false);

    ui_.seriesTable->setColumnCount(5);
    headers.clear();
    headers << tr("Series #") 
            << tr("Series Description") 
            << tr("Modality")
            << tr("Size")
            << tr("Count")
            ;
    ui_.seriesTable->setHorizontalHeaderLabels(headers);
    ui_.seriesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_.seriesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_.seriesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui_.seriesTable->verticalHeader()->setVisible(false);

    updateFindResult();    
    initTableContent();
    ui_.patientTable->resizeColumnsToContents();
    ui_.studyTable->resizeColumnsToContents();
    ui_.seriesTable->resizeColumnsToContents();
    connect(ui_.patientTable, &QTableWidget::itemSelectionChanged, this, &DICOMReaderDlg::slotPatientTableItemSelectionChanged);
    connect(ui_.studyTable, &QTableWidget::itemSelectionChanged, this, &DICOMReaderDlg::slotStudyTableItemSelectionChanged);
    connect(ui_.seriesTable, &QTableWidget::itemSelectionChanged, this, &DICOMReaderDlg::slotSeriesTableItemSelectionChanged);
    ui_.loadBtn->setEnabled(false);
    connect(ui_.loadBtn, &QAbstractButton::pressed, this, &DICOMReaderDlg::slotLoadBtnPressed);
    connect(ui_.cancelBtn, &QAbstractButton::clicked, this, &QDialog::reject);
    connect(pd_, &DICOMReader::languageChanged, this, &DICOMReaderDlg::slotLangChanged);

    qDebug() << "DICOMReaderDlg::DICOMReaderDlg called\n";
}

DICOMReaderDlg::~DICOMReaderDlg()
{
    qDebug() << "DICOMReaderDlg::~DICOMReaderDlg called\n";
}

void DICOMReaderDlg::slotLoadBtnPressed()
{
    qDebug() << "DICOMReaderDlg::slotLoadBtnPressed called\n";
    int row_id = ui_.seriesTable->currentRow();    
    bool ok = false;
    auto selected_series_id = ui_.seriesTable->item(row_id, 0)->data(Qt::UserRole).toInt(&ok);
    if(!ok){
        return;
    }
    qDebug() << "Selected series id:" <<selected_series_id << "\n";
    auto series_filenames = file_finder_->GetFileNamesForSeries(selected_series_id);
    if(series_filenames->GetNumberOfValues() < 1){
        qWarning() << "No file images found in series id:" << selected_series_id <<"\n";
        return;
    }
    cc_ = vtkSmartPointer<vtkCallbackCommand>::New();
    cc_->SetCallback(&DICOMReaderDlg::ReadProgressHandler);
    cc_->SetClientData(this);
    dicom_reader_ = vtkSmartPointer<vtkDICOMReader>::New();        
    dicom_reader_->AddObserver(vtkCommand::ProgressEvent, cc_);
    dicom_reader_->SetFileNames(series_filenames);
    dicom_reader_->Update();
    output_data_ = dicom_reader_->GetOutput();

    optimizeImageData();
    
    pd_->onSeriesReadFinished();

    accept();
}

void DICOMReaderDlg::slotPatientTableItemSelectionChanged()
{
    int p_id = ui_.patientTable->currentRow();
    qDebug() << "Current patient id:" << p_id << "\n";
    ui_.studyTable->clearContents();   
    ui_.studyTable->setRowCount(0);
    auto study_ids = file_finder_->GetStudiesForPatient(p_id);
    if(study_ids == nullptr || study_ids->GetNumberOfValues() < 1){
        return;
    }
    ui_.studyTable->setRowCount(study_ids->GetNumberOfValues());
    for(auto i = 0; i < study_ids->GetNumberOfValues(); ++i){
        int study_id = study_ids->GetValue(i);
        const auto& study_meta_db = file_finder_->GetStudyRecord(study_id);

        const auto& sd_v = study_meta_db.Get(DC::StudyDate);
        QTableWidgetItem* new_cell = nullptr;
        if(sd_v.IsValid()){
            new_cell = new QTableWidgetItem(sd_v.AsUTF8String().c_str());            
        }else{
            new_cell = new QTableWidgetItem("---");            
        }
        new_cell->setTextAlignment(Qt::AlignCenter);
        ui_.studyTable->setItem(i, 0, new_cell);

        const auto& id_v = study_meta_db.Get(DC::StudyID);
        if(id_v.IsValid()){
            new_cell = new QTableWidgetItem(id_v.AsUTF8String().c_str());    
        }else{
            new_cell = new QTableWidgetItem("---");       
        }
        new_cell->setTextAlignment(Qt::AlignCenter);
        ui_.studyTable->setItem(i, 1, new_cell);

        const auto& sdes_v = study_meta_db.Get(DC::StudyDescription);
        if(sdes_v.IsValid()){
            new_cell = new QTableWidgetItem(sdes_v.AsUTF8String().c_str());            
        }else{
            new_cell = new QTableWidgetItem("---");            
        }
        new_cell->setTextAlignment(Qt::AlignCenter);
        ui_.studyTable->setItem(i, 2, new_cell);
        
        auto series_number = file_finder_->GetLastSeriesForStudy(study_id) - file_finder_->GetFirstSeriesForStudy(study_id) + 1;
        new_cell = new QTableWidgetItem(QString::number(series_number));            
        new_cell->setTextAlignment(Qt::AlignCenter);
        ui_.studyTable->setItem(i, 3, new_cell);
    }
    ui_.studyTable->resizeColumnsToContents();
    ui_.loadBtn->setEnabled(false);
}

void DICOMReaderDlg::slotStudyTableItemSelectionChanged()
{
    int s_id = ui_.studyTable->currentRow();
    qDebug() << "Current Study id:" << s_id << "\n";
    ui_.seriesTable->clearContents();   
    ui_.seriesTable->setRowCount(0);
    auto first_series_id = file_finder_->GetFirstSeriesForStudy(s_id);
    auto last_series_id = file_finder_->GetLastSeriesForStudy(s_id);
    ui_.seriesTable->setRowCount(last_series_id - first_series_id + 1);
    for(auto i = first_series_id; i <= last_series_id; ++i){
        auto series_meta_db = file_finder_->GetMetaDataForSeries(i);
        const auto& sn_v = series_meta_db->Get(DC::SeriesNumber);
        QTableWidgetItem* new_cell = nullptr;
        if(sn_v.IsValid()){
            new_cell = new QTableWidgetItem(sn_v.AsUTF8String().c_str());            
        }else{
            new_cell = new QTableWidgetItem("---");            
        }
        new_cell->setTextAlignment(Qt::AlignCenter);
        new_cell->setData(Qt::UserRole, i);
        ui_.seriesTable->setItem(i, 0, new_cell);

        const auto& sd_v = series_meta_db->Get(DC::SeriesDescription);
        if(sd_v.IsValid()){
            new_cell = new QTableWidgetItem(sd_v.AsUTF8String().c_str());    
        }else{
            new_cell = new QTableWidgetItem("---");            
        }
        new_cell->setTextAlignment(Qt::AlignCenter);
        ui_.seriesTable->setItem(i, 1, new_cell);

        const auto& sm_v = series_meta_db->GetAttributeValue(DC::Modality);
        if(sm_v.IsValid()){
            new_cell = new QTableWidgetItem(sm_v.AsUTF8String().c_str());              
        }else{
            new_cell = new QTableWidgetItem("---");            
        }
        new_cell->setTextAlignment(Qt::AlignCenter);
        ui_.seriesTable->setItem(i, 2, new_cell);

        QString rows_str = series_meta_db->GetAttributeValue(DC::Rows).AsUTF8String().c_str();
        QString cols_str = series_meta_db->GetAttributeValue(DC::Columns).AsUTF8String().c_str();
        QString size_str = QString("%1x%2").arg(rows_str, cols_str);
        new_cell = new QTableWidgetItem(size_str);            
        new_cell->setTextAlignment(Qt::AlignCenter);
        ui_.seriesTable->setItem(i, 3, new_cell);

        auto filenames = file_finder_->GetFileNamesForSeries(i);
        new_cell = new QTableWidgetItem(QString::number(filenames->GetNumberOfValues()));            
        new_cell->setTextAlignment(Qt::AlignCenter);
        ui_.seriesTable->setItem(i, 4, new_cell);
    }
    ui_.seriesTable->resizeColumnsToContents();
    ui_.loadBtn->setEnabled(false);
}

void DICOMReaderDlg::slotSeriesTableItemSelectionChanged()
{
    int row_id = ui_.seriesTable->currentRow();    
    auto selected_series_id = ui_.seriesTable->item(row_id, 0)->data(Qt::UserRole).toInt();
    qDebug() << "Current Series id:" << selected_series_id << "\n";
    ui_.loadBtn->setEnabled(true);
}

void DICOMReaderDlg::slotLangChanged()
{
    ui_.loadBtn->setText(tr("Load"));
    ui_.cancelBtn->setText(tr("Cancel"));
}

void DICOMReaderDlg::ReadProgressHandler(vtkObject* caller, unsigned long eid, void* client_data, void* calldata)
{
    auto algorithm = static_cast<vtkAlgorithm*>(caller);
    double p = algorithm->GetProgress();   // 0.0 ~ 1.0
    const char* s = algorithm->GetProgressText();
    auto dlg_obj = reinterpret_cast<DICOMReaderDlg*>(client_data);
    if(dlg_obj){
        dlg_obj->handleProgress(p * 100.0, s);
    }
}

void DICOMReaderDlg::handleProgress(double p, const QString& text)
{
    ui_.progressBar->setValue(static_cast<int>(p));
    ui_.progressLabel->setText(text);
}

void DICOMReaderDlg::initTableContent()
{
    auto patient_number = file_finder_->GetNumberOfPatients();
    ui_.patientTable->setRowCount(patient_number);
    for(auto p_id = 0; p_id < patient_number; ++p_id){
        const auto& db = file_finder_->GetPatientRecord(p_id);
        const auto& val = db.Get(DC::PatientName);
        QString name = "---";
        if(val.IsValid()){
            name = QString::fromUtf8(val.AsUTF8String().c_str());
        }
        auto new_cell = new QTableWidgetItem(name);
        new_cell->setTextAlignment(Qt::AlignCenter);
        ui_.patientTable->setItem(p_id, 0, new_cell);

        QString id = "---";
        const auto& id_v = db.Get(DC::PatientID);
        if(id_v.IsValid()){
            id = QString::fromUtf8(id_v.AsUTF8String().c_str());
        }
        new_cell = new QTableWidgetItem(id);
        new_cell->setTextAlignment(Qt::AlignCenter);
        ui_.patientTable->setItem(p_id, 1, new_cell);

        QString birth_data = "---";
        const auto& bd_v = db.Get(DC::PatientBirthDate);
        if(bd_v.IsValid()){
            birth_data = QString::fromUtf8(bd_v.AsUTF8String().c_str());
        }
        new_cell = new QTableWidgetItem(birth_data);
        new_cell->setTextAlignment(Qt::AlignCenter);
        ui_.patientTable->setItem(p_id, 2, new_cell);        

        QString sex = "---";
        const auto& sex_v = db.Get(DC::PatientSex);
        if(sex_v.IsValid()){
            sex = QString::fromUtf8(sex_v.AsUTF8String().c_str());
        }
        new_cell = new QTableWidgetItem(sex);
        new_cell->setTextAlignment(Qt::AlignCenter);
        ui_.patientTable->setItem(p_id, 3, new_cell);

        int number_of_studies = 0;
        auto studies_v = file_finder_->GetStudiesForPatient(p_id);
        if(studies_v){
            number_of_studies = studies_v->GetNumberOfValues();
        }
        new_cell = new QTableWidgetItem(QString::number(number_of_studies));
        new_cell->setTextAlignment(Qt::AlignCenter);
        ui_.patientTable->setItem(p_id, 4, new_cell);

        int last_study_id = studies_v->GetValue(studies_v->GetNumberOfValues() - 1);
        const auto& last_study_v = file_finder_->GetStudyRecord(last_study_id);
        const auto& sd_v = last_study_v.Get(DC::StudyDate);
        if(sd_v.IsValid()){
            new_cell = new QTableWidgetItem(QString::fromUtf8(sd_v.AsUTF8String().c_str()));
        }else{
            new_cell = new QTableWidgetItem("--");
        }
        new_cell->setTextAlignment(Qt::AlignCenter);
        ui_.patientTable->setItem(p_id, 5, new_cell);       
    }
}

void DICOMReaderDlg::updateFindResult()
{
    QFileInfo fi(pd_->dicomDirectory());
    if(!fi.exists() || !fi.isDir()){
        //QMessageBox::warning(this, tr("Load error"), tr("Target directory not exists!"), QMessageBox::Ok);
        return;
    }
    file_finder_->SetDirectoryName(pd_->dicomDirectory().toUtf8().toStdString().c_str());
    file_finder_->SetFindLevelToSeries();
    file_finder_->SetQueryFilesToAlways();
    file_finder_->IgnoreDicomdirOn();
    file_finder_->Update();
}

void DICOMReaderDlg::optimizeImageData()
{
    AdjustImageOrientation(output_data_, dicom_reader_);
    CenterImage(output_data_);
}

std::optional<_tagDICOMMetaData> DICOMReaderDlg::readMetaData() const
{
    if(dicom_reader_ == nullptr){
        return std::nullopt;
    }
    _tagDICOMMetaData md;
    auto meta_data = dicom_reader_->GetMetaData();
    auto dicom_value = meta_data->GetAttributeValue(kPatientNameTag);
    md.patient_name = dicom_value.AsUTF8String();
    dicom_value = meta_data->GetAttributeValue(kPatientAgeTag);
    md.patient_age = dicom_value.AsInt();
    dicom_value = meta_data->GetAttributeValue(kPatientSexTag);
    md.patient_gender = dicom_value.AsUTF8String();
    dicom_value = meta_data->GetAttributeValue(kAcquisitionDateTag);
    md.acquistion_date = dicom_value.AsUTF8String();
    dicom_value = meta_data->GetAttributeValue(kAcquisitionTimeTag);
    md.acquistion_time = dicom_value.AsUTF8String();
    dicom_value = meta_data->GetAttributeValue(kManufacturerTag);
    md.manufacturer = dicom_value.AsUTF8String();
    dicom_value = meta_data->GetAttributeValue(kWindowWidthTag);
    md.win_width = dicom_value.AsDouble();
    dicom_value = meta_data->GetAttributeValue(kWindowCenterTag);
    md.win_level = dicom_value.AsDouble();
    dicom_value = meta_data->GetAttributeValue(kSliceThicknessTag);
    md.slice_thickness = dicom_value.AsDouble();
    return md;
}

void DICOMReaderDlg::done(int r)
{
    pd_->onViewClosed();
    QDialog::done(r);
}