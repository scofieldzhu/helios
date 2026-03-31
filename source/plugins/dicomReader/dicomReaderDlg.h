/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2026)
* author: zhucg
* time:2025/12/2
*******************************************************/
#ifndef __dicomReaderDlg_h__
#define __dicomReaderDlg_h__

#include "ui_dicomReaderDlg.h"
#include <vtkDICOMDirectory.h>
#include <vtkDICOMReader.h>
#include <vtkSmartPointer.h>
#include <vtkCallbackCommand.h>
#include "IDICOMReader.h"

class DICOMReader;

class DICOMReaderDlg : public QDialog
{
	Q_OBJECT
public:
    std::optional<_tagDICOMMetaData> readMetaData()const;
    vtkSmartPointer<vtkImageData> getImageData(){ return output_data_; }
	DICOMReaderDlg(QWidget* p, DICOMReader* pd);
	~DICOMReaderDlg();

private slots:
    void slotLoadBtnPressed();
    void slotPatientTableItemSelectionChanged();
    void slotStudyTableItemSelectionChanged();
    void slotSeriesTableItemSelectionChanged();
    void slotLangChanged();

private:
    static void ReadProgressHandler(vtkObject* caller, unsigned long eid, void* clientdata, void* calldata);
    void handleProgress(double p, const QString& text);
	void initTableContent();
    void updateFindResult();
    void optimizeImageData();
    void done(int r) override;
	Ui::Dialog ui_;
	DICOMReader* pd_;
    vtkSmartPointer<vtkDICOMDirectory> file_finder_;
    vtkSmartPointer<vtkDICOMReader> dicom_reader_;
    vtkSmartPointer<vtkImageData> output_data_;
    vtkSmartPointer<vtkCallbackCommand> cc_;
};

#endif