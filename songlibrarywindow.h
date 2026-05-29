#ifndef SONGLIBRARYWINDOW_H
#define SONGLIBRARYWINDOW_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QFileDialog>

class SongLibraryWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SongLibraryWindow(QWidget *parent = nullptr);
    ~SongLibraryWindow();

signals:
    void songSelected(const QList<QUrl> &songList, int selectedIndex); // 选完歌曲点击"确定"后的信号

private slots:
    void on_importBtn_clicked();    // 导入歌曲
    void on_confirmBtn_clicked();   // 确定选择

private:
    QListWidget *songList; // 歌曲列表控件（纯代码创建）
};

#endif // SONGLIBRARYWINDOW_H