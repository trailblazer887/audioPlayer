#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QAudioOutput>
#include <QTimer>
#include <QList>
#include <QUrl>
#include <QMessageBox>
#include <QObject>

// 前向声明歌库窗口（避免循环包含）
class SongLibraryWindow;

// UI类（由mainwindow.ui自动生成）
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_playPauseBtn_clicked();       // 播放/暂停
    void on_progressSlider_sliderMoved(int position); // 进度条拖动
    void on_progressSlider_sliderReleased(); // 进度条点击
    void updateProgressBar();             // 实时更新进度条
    void on_openLibraryBtn_clicked();     // 打开歌库
    void on_prevSongBtn_clicked();        // 上一曲（手动）
    void on_nextSongBtn_clicked();        // 下一曲（手动）
    void on_autoNextBtn_clicked();        // 是否自动切换
    void handleSongSelected(const QList<QUrl> &newSongList, int selectedIndex); // 处理选中歌曲

private:
    Ui::MainWindow *ui;                 // 主窗口UI指针
    SongLibraryWindow *songLibWindow;   // 歌库窗口指针
    QMediaPlayer *player;               // 播放器核心

    QAudioOutput *audioOutput;

    QList<QUrl> songList;               // 手动播放列表（存储歌曲路径）
    int currentSongIndex;               // 当前播放下标（-1=无歌曲）
    QTimer *progressTimer;              // 进度条定时器
    bool autoNextEnabled;               // 是否启用自动切换下一曲
    QLabel *timePlayedLabel;  // 已播放时间标签
    QLabel *timeTotalLabel;   // 总时间标签

protected:
    bool eventFilter(QObject *obj, QEvent *event) override; // 过滤器
};

#endif // MAINWINDOW_H