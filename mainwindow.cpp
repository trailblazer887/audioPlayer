#include "mainwindow.h"
#include "ui_mainwindow.h"       // 自动生成的UI头文件
#include "songlibrarywindow.h"   // 歌库窗口头文件
#include <QKeyEvent>
#include <QMouseEvent>
#include <QStyle>
#include <QStyleOption>
#include <QLabel>
#include <QFileInfo>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    songLibWindow(nullptr),
    currentSongIndex(-1),
    progressTimer(new QTimer(this)),
    autoNextEnabled(true)
{
    ui->setupUi(this); // 加载主窗口UI
    // 创建“当前播放”标签，置于顶部居中
    nowPlayingLabel = new QLabel("当前播放：\n", this);
    nowPlayingLabel->setAlignment(Qt::AlignCenter);
    nowPlayingLabel->setStyleSheet("color: white; font-size: 14px;");
    nowPlayingLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    nowPlayingLabel->setGeometry(0, 10, width(), 40);



    this->setFixedSize(847, 558); // 固定窗口大小，禁止拉伸
    // 创建时间显示标签
    timePlayedLabel = new QLabel("00:00", this);
    timeTotalLabel = new QLabel("/ 00:00", this);
    timePlayedLabel->setStyleSheet("color: white; background: transparent;");
    timeTotalLabel->setStyleSheet("color: white; background: transparent;");
    // 等宽字体避免时间跳动
    QFont monoFont("Consolas", 10);
    timePlayedLabel->setFont(monoFont);
    timeTotalLabel->setFont(monoFont);
    statusBar()->addPermanentWidget(timePlayedLabel);
    statusBar()->addPermanentWidget(timeTotalLabel);

    ui->autoNextBtn->setText("关闭自动切换"); // 自动切换按钮初始化

    qApp->installEventFilter(this); // 安装事件过滤器

    // 初始化播放器
    player = new QMediaPlayer(this);

    audioOutput = new QAudioOutput(this); // 音频播放器
    player->setAudioOutput(audioOutput);  // 把播放器绑定到音频输出
    audioOutput->setVolume(1.0);

    // 初始化控件
    ui->playPauseBtn->setText("播放"); // 按钮文字为"播放"
    ui->progressSlider->setRange(0, 1000); // 初始化进度条
    ui->progressSlider->setValue(0);
    ui->prevSongBtn->setEnabled(false); // 上/下一曲按钮关闭
    ui->nextSongBtn->setEnabled(false);
    ui->progressSlider->setSingleStep(40); // 键盘改变进度条的幅度

    // 定时器：每500ms更新进度条
    progressTimer->setInterval(500);
    connect(progressTimer, &QTimer::timeout, this, &MainWindow::updateProgressBar);

    // 播放状态改变时更新按钮文本
    connect(player, &QMediaPlayer::playbackStateChanged, this, [=](QMediaPlayer::PlaybackState state){
        if (state == QMediaPlayer::PlayingState) {
            ui->playPauseBtn->setText("暂停");
            progressTimer->start();
        } else {
            ui->playPauseBtn->setText("播放");
            if (state == QMediaPlayer::StoppedState) {
                progressTimer->stop();
                ui->progressSlider->setValue(0);
                nowPlayingLabel->setText("当前播放：\n");
            }
        }
    });

    // 歌曲播放完后自动切换下一曲
    connect(player, &QMediaPlayer::mediaStatusChanged, this, [=](QMediaPlayer::MediaStatus status){
        if (status == QMediaPlayer::EndOfMedia && !songList.isEmpty() && autoNextEnabled){
            on_nextSongBtn_clicked();
        }
    });

    // 创建命令行输入框（隐藏）
    commandLine = new QLineEdit(this);
    commandLine->setFont(QFont("Consolas", 12));
    commandLine->setStyleSheet("background-color: black; color: white; border: 1px solid gray;");
    commandLine->setGeometry(10, 10, 300, 30); // 左上角固定位置
    commandLine->hide();
    connect(commandLine, &QLineEdit::returnPressed, this, [this]() {
        processCommand(commandLine->text());
        commandLine->clear();
        commandLine->hide();
    });

    // 延迟初始化标记条（确保 progressSlider 布局完成）
    QTimer::singleShot(0, this, &MainWindow::initMarkerBar);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 播放/暂停
void MainWindow::on_playPauseBtn_clicked()
{
    if (songList.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先从歌库选择歌曲！");
        return;
    }
    if (player->playbackState() == QMediaPlayer::PlayingState) {
        player->pause();
    } else {
        player->play();
        updateNowPlayingLabel(); // 加载标签
    }
}

// 进度条拖动
void MainWindow::on_progressSlider_sliderMoved(int position)
{
    if (player->duration() > 0) {
        qint64 newPos = (player->duration() * position) / 1000;
        player->setPosition(newPos);
    }
}

// 进度条 点击/拖动结束 后跳转进度
void MainWindow::on_progressSlider_sliderReleased()
{
    int position = ui->progressSlider->value();
    if (player->duration() > 0) {
        qint64 newPos = (player->duration() * position) / 1000;
        player->setPosition(newPos);
    }
}

// 实时更新进度条(由"定时器"驱动)
void MainWindow::updateProgressBar()
{
    if (player->duration() > 0) {
        int progress = (player->position() * 1000) / player->duration();
        ui->progressSlider->setValue(progress);
        // 更新时间标签
        qint64 position = player->position();
        qint64 duration = player->duration();
        auto formatTime = [](qint64 ms) {
            int secs = ms / 1000;
            int min = secs / 60;
            int sec = secs % 60;
            return QString("%1:%2").arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0'));
        };
        timePlayedLabel->setText(formatTime(position));
        timeTotalLabel->setText("/ " + formatTime(duration));
    } else {
        timePlayedLabel->setText("00:00");
        timeTotalLabel->setText("/ 00:00");
    }
}

// 打开歌库
void MainWindow::on_openLibraryBtn_clicked()
{
    if (!songLibWindow) { // 如果曾经没有打开歌库
        songLibWindow = new SongLibraryWindow(this); // 新建歌库窗口
        connect(songLibWindow, &SongLibraryWindow::songSelected, this, &MainWindow::handleSongSelected); // 接收到songSelected信号后调用函数处理
    }
    songLibWindow->show(); // 展示窗口
}

// 处理歌库选中的歌曲(当收到songSelected信号时)
void MainWindow::handleSongSelected(const QList<QUrl> &newSongList, int selectedIndex)
{
    for (const QUrl &url : newSongList) { // 合并新播放列表(去重)
        if (!songList.contains(url)){
            songList.append(url);
        }
    }

    if (selectedIndex >= 0 && selectedIndex < newSongList.size()){ // 将newSongList中的索引转换为songList中的索引
        QUrl selectedUrl = newSongList.at(selectedIndex);
        currentSongIndex = songList.indexOf(selectedUrl);
    }else {
        currentSongIndex = 0;
    }

    if (currentSongIndex >= 0 && currentSongIndex < songList.size()){ // 加载并播放音乐
        player->setSource(songList[currentSongIndex]);
        player->play();
    }

    ui->prevSongBtn->setEnabled(!songList.isEmpty()); // 上/下一曲按钮激活
    ui->nextSongBtn->setEnabled(!songList.isEmpty());
    updateNowPlayingLabel(); // 更新"当前播放标签"
}

// 手动上一曲
void MainWindow::on_prevSongBtn_clicked()
{
    if (songList.isEmpty()) return;

    currentSongIndex = (currentSongIndex - 1 + songList.size()) % songList.size();
    player->setSource(songList[currentSongIndex]);
    player->play();
    updateNowPlayingLabel(); // 加载标签
}

// 手动下一曲
void MainWindow::on_nextSongBtn_clicked()
{
    if (songList.isEmpty()) return;

    currentSongIndex = (currentSongIndex + 1 + songList.size()) % songList.size();
    player->setSource(songList[currentSongIndex]);
    player->play();
    updateNowPlayingLabel(); // 加载标签
}


// 事件处理 Filter
bool MainWindow::eventFilter(QObject *obj, QEvent *event){
    // 命令行可见时：只处理 Esc 隐藏，其余放行
    if (commandLine && commandLine->isVisible()) {
        if (event->type() == QEvent::KeyPress) {
        QKeyEvent *k = static_cast<QKeyEvent*>(event);
        if (k->key() == Qt::Key_Escape) {
            commandLine->hide();
            return true;
        }
        }
        // 其他事件不拦截，让命令行正常输入
        return QMainWindow::eventFilter(obj, event);
    }

    // 唤起命令行的快捷键
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *k = static_cast<QKeyEvent*>(event);
        if (k->key() == Qt::Key_Colon) {  // Shift + ;即 ':'
            if (commandLine) {
                commandLine->show();
                commandLine->setFocus();
            }
            return true;
        }
    }

    // 标记箭头点击跳转
    if (event->type() == QEvent::MouseButtonPress) {
        QLabel *arrow = qobject_cast<QLabel*>(obj);
        if (arrow && arrow->parent() == markerBar) {
        bool ok;
        int idx = arrow->property("markerIndex").toInt(&ok);
        if (ok && idx >= 0 && idx < markers.size()) {
            qint64 pos = markers[idx].pos;
            player->setPosition(pos);
            // 同步进度条和标签（updateProgressBar 会刷新，但为了即时反馈可主动调用）
            if (player->duration() > 0) {
            int sliderVal = (int)(pos * 1000 / player->duration());
            ui->progressSlider->setValue(sliderVal);
            }
        }
        return true;
        }
    }
    // 点击进度条轨道直接跳转
    if (obj == ui->progressSlider && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            QStyleOptionSlider opt;
            opt.initFrom(ui->progressSlider);
            // 补充滑块当前值信息，否则 handleRect 计算不准确
            opt.minimum = ui->progressSlider->minimum();
            opt.maximum = ui->progressSlider->maximum();
            opt.sliderPosition = ui->progressSlider->sliderPosition();
            opt.sliderValue = ui->progressSlider->value();
            opt.orientation = ui->progressSlider->orientation();
            opt.subControls = QStyle::SC_SliderHandle;

            QRect handleRect = ui->progressSlider->style()->subControlRect(
                QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, ui->progressSlider);
            // 如果点击位置不在手柄上 → 轨道点击跳转
            if (!handleRect.contains(mouseEvent->pos())) {
                int value = QStyle::sliderValueFromPosition(
                    ui->progressSlider->minimum(),
                    ui->progressSlider->maximum(),
                    mouseEvent->pos().x(),
                    ui->progressSlider->width());
                ui->progressSlider->setValue(value);
                on_progressSlider_sliderReleased();
                return true;
            }
            // 否则执行原逻辑
        }
    }
    // 拦截进度条的上下键，防止改变值
    if (obj == ui->progressSlider && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down) {
            return true;
        }
    }

    if (event->type() == QEvent::KeyPress){ // 识别事件是否是键盘按下
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Right){ // 如果是右方向键
            if (player->playbackState() == QMediaPlayer::StoppedState){ // 如果是没有歌曲正在播放或暂停(没有选择歌曲)
                return true;
            }
            qint64 currentPos = player->position();
            qint64 newPos = currentPos + 6000; // 若右方向键合法，歌曲加6000毫秒(6秒)
            if (newPos > player->duration()){ // 防止加后溢出
                newPos = player->duration();
            }
            player->setPosition(newPos);
            int sliderValue = static_cast<int>(newPos * 1000 / player->duration());
            ui->progressSlider->setValue(sliderValue);
            return true;
        }else if (keyEvent->key() == Qt::Key_Left){ // 如果是左方向键
            if (player->playbackState() == QMediaPlayer::StoppedState){ // 如果是没有歌曲正在播放或暂停(没有选择歌曲)
                return true;
            }
            qint64 currentPos = player->position();
            qint64 newPos = currentPos - 6000; // 若左方向键合法，歌曲减6000毫秒(6秒)
            if (newPos < 0){
                newPos = 0;
            }
            player->setPosition(newPos);
            int sliderValue = static_cast<int>(newPos * 1000 / player->duration());
            ui->progressSlider->setValue(sliderValue);
            return true;
        }else if (keyEvent->key() == Qt::Key_Space){ // 空格键控制播放按钮
            on_playPauseBtn_clicked();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event); // 其他事件默认处理;
}

// 切换播放状态
void MainWindow::on_autoNextBtn_clicked(){
    autoNextEnabled = !autoNextEnabled;
    if (autoNextEnabled){
        ui->autoNextBtn->setText("关闭自动切换");
    }else{
        ui->autoNextBtn->setText("开启自动切换");
    }
}

// 实现标记容器初始化函数
void MainWindow::initMarkerBar()
{
    if (!ui->progressSlider || !ui->progressSlider->isVisible()) return;

    QPoint topLeft = ui->progressSlider->mapTo(this, QPoint(0, 0));
    QSize size = ui->progressSlider->size();
    if (size.width() <= 0 || size.height() <= 0) return;

    int barHeight = 20;
    markerBar = new QWidget(this);
    markerBar->setStyleSheet("background: transparent;");
    markerBar->setGeometry(topLeft.x(), topLeft.y() - barHeight, size.width(), barHeight);
    markerBar->show();
}
// 重建标记可视化
void MainWindow::rebuildMarkers()
{

    if (!markerBar) return;

    // 清除旧标记
    qDeleteAll(markerBar->findChildren<QLabel*>());

    if (!markerBar->isVisible() || markerBar->width() <= 0) return; // 严谨检查
    if (player->duration() <= 0) return;

    int barWidth = markerBar->width();
    for (int i = 0; i < markers.size(); ++i) {
        const Marker &m = markers[i];
        double ratio = (double)m.pos / player->duration();
        int x = (int)(ratio * barWidth);

        QLabel *arrow = new QLabel("▼", markerBar);
        arrow->setStyleSheet("color: red; font-size: 14px; background: transparent;");
        arrow->setToolTip(m.label);
        arrow->setAlignment(Qt::AlignCenter);
        arrow->setFixedSize(20, markerBar->height());
        arrow->move(x - 10, 0); // 箭头居中
        arrow->setProperty("markerIndex", i); // 记录索引
        arrow->installEventFilter(this);      // 监听点击
        arrow->show();
    }
}

// 添加标记的实现
void MainWindow::addMarker(const QString &label)
{
    if (player->duration() <= 0) return; // 没有歌曲时不添加

    qint64 pos = player->position();
    // 按时间顺序插入
    int insertIdx = 0;
    for (; insertIdx < markers.size(); ++insertIdx) {
        if (pos < markers[insertIdx].pos) break;
    }
    markers.insert(insertIdx, {pos, label});
    rebuildMarkers();
}

// 删除标记的实现
void MainWindow::delMarker(int id)
{
    if (id < 1 || id > markers.size()) return;
    markers.removeAt(id - 1);
    rebuildMarkers();
}

//命令行处理
void MainWindow::processCommand(const QString &text)
{
    QString trimmed = text.trimmed();
    if (trimmed.startsWith("set ", Qt::CaseInsensitive)) { // set
        QString label = trimmed.mid(4).trimmed();
        if (!label.isEmpty()) {
            addMarker(label);
        }
    }else if (trimmed.startsWith("del ", Qt::CaseInsensitive)) { // del
        QString arg = trimmed.mid(4).trimmed();
        bool ok;
        int id = arg.toInt(&ok);
        if (ok) {
            // 按标识符删除
            delMarker(id);
        }else {
            for (int i = 0; i < markers.size(); ++i) {
                if (markers[i].label == arg) {
                    delMarker(i + 1);          // 按标签删除（标识符从1开始）
                    break;
                }
            }
        }
    }else if (trimmed.compare("clear", Qt::CaseInsensitive) == 0) { // clear
        markers.clear();
        rebuildMarkers();
    }
}

// 更新"当前播放:"标签
void MainWindow::updateNowPlayingLabel()
{
    if (currentSongIndex >= 0 && currentSongIndex < songList.size()) {
        QString filePath = player->source().toLocalFile();
        if (filePath.isEmpty()) {
            filePath = player->source().toString();
        }
        QFileInfo fi(filePath);
        QString name = fi.completeBaseName(); // 去掉后缀的文件名
        nowPlayingLabel->setText("当前播放：\n" + name);
    } else {
        nowPlayingLabel->setText("当前播放：\n");
    }
}