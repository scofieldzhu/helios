/******************************************************** 
* author: scofieldzhu
* time:2025/12/2
*******************************************************/
#include "dicomReader.h"
#include <QTranslator>
#include <QLocale>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include "dicomReaderDlg.h"
#include "interfaces/IViewerEventSink.h"

DICOMReader::DICOMReader(QObject* p /*= nullptr*/)
	:QObject(p)
{
    lang_translator_ = new QTranslator(this);
    QString translation_file_path = ":/translations/DICOMReader_" + QLocale::system().name() + ".qm";
    if(lang_translator_->load(translation_file_path)){
        current_lang_code_ = QLocale::system().name();
        QCoreApplication::installTranslator(lang_translator_);
        qDebug() << "DICOMReader plugin translation installed successfully!\n";
    }else{
        qDebug() << "DICOMReader plugin translation installed failed!\n";
    }
}

DICOMReader::~DICOMReader()
{
	delete dlg_;
}

void DICOMReader::onSeriesReadFinished()
{
	if(notifier_){
		notifier_(this);
	}	
}

void DICOMReader::setDICOMDirectory(const QString& dir)
{
    dicom_dir_ = dir;
}

vtkSmartPointer<vtkImageData> DICOMReader::getSeriesData()
{
    return dlg_->getImageData();
}

bool DICOMReader::readSeriesData(int series_id)
{
	return false;
}

std::optional<_tagDICOMMetaData> DICOMReader::readMetaData() const
{
    return dlg_->readMetaData();
}

bool DICOMReader::createView(QWidget* parent)
{
	if(dlg_ == nullptr){
		dlg_ = new DICOMReaderDlg(parent, this);
	}
    if(event_sink_){
        event_sink_->onViewerStateChanged(this, "ViewOpen");
    }
    dlg_->exec();
	return true;
}

QWidget* DICOMReader::getViewWidget()
{
	return dlg_;
}

void DICOMReader::switchLanguage(const QString& lang_code)
{
    if(lang_code == current_lang_code_){
        return;
    }
    QCoreApplication::removeTranslator(lang_translator_);
    QString translation_file_path = ":/translations/DICOMReader_" + lang_code + ".qm";
    if(QFile::exists(translation_file_path) && lang_translator_->load(translation_file_path)){
        current_lang_code_ = lang_code;
        QCoreApplication::installTranslator(lang_translator_);
        emit languageChanged();
    }
}

void DICOMReader::onViewClosed()
{
    if(event_sink_){
        event_sink_->onViewerStateChanged(this, "ViewClose");
    }
}

void DICOMReader::setEventSink(IViewerEventSink* sink)
{
    event_sink_ = sink;
}