/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2026)
* author: zhucg
* time:2025/12/2
*******************************************************/
#ifndef __dicomReader_h__
#define __dicomReader_h__

#include "IViewer.h"
#include "IDICOMReader.h"

class DICOMReaderDlg;
class QTranslator;

class DICOMReader : public QObject, public IDICOMReader, public IViewer
{
	Q_OBJECT
	Q_INTERFACES(IDICOMReader IViewer)
	Q_PLUGIN_METADATA(IID IDICOM_READER_IID FILE "dicomReader.json")

signals:
    void languageChanged();

public:	
    void onSeriesReadFinished();
    void setDICOMDirectory(const QString& dir) override;
    const QString& dicomDirectory()const{ return dicom_dir_; }
    void setFinishReadCallback(CallbackType cb) override{ notifier_ = cb; }
    vtkSmartPointer<vtkImageData> getSeriesData() override;
    bool readSeriesData(int series_id) override;
    std::optional<_tagDICOMMetaData> readMetaData()const override;
	bool createView(QWidget* parent) override;
    QWidget* getViewWidget() override;   
    void switchLanguage(const QString &lang_code) override;
    void onViewClosed() override;
    void setEventSink(IViewerEventSink* sink) override;
    DICOMReader(QObject* p = nullptr);
	~DICOMReader();

private:
	DICOMReaderDlg* dlg_ = nullptr;
    QString dicom_dir_;
    CallbackType notifier_;
    QTranslator* lang_translator_;
    QString current_lang_code_;
    IViewerEventSink* event_sink_ = nullptr;
};

#endif