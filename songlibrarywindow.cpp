#include "songlibrarywindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>

SongLibraryWindow::SongLibraryWindow(QWidget *parent) :
    QDialog(parent),
    songList(new QListWidget(this)) // 这里的songList是窗口列表控件
{
    // 窗口标题，尺寸
    this->setWindowTitle("歌曲库");
    this->resize(400, 300);

    // 创建按钮
    QPushButton *importBtn = new QPushButton("导入歌曲", this);
    QPushButton *confirmBtn = new QPushButton("确定", this);
    QPushButton *cancelBtn = new QPushButton("取消", this);

    // 布局（纯代码）
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *btnLayout = new QHBoxLayout();
    mainLayout->addWidget(songList);
    btnLayout->addWidget(importBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(confirmBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);
    this->setLayout(mainLayout);

    // 信号槽
    connect(importBtn, &QPushButton::clicked, this, &SongLibraryWindow::on_importBtn_clicked);
    connect(confirmBtn, &QPushButton::clicked, this, &SongLibraryWindow::on_confirmBtn_clicked);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::close);

    // 初始禁用确定按钮
    confirmBtn->setEnabled(false);

    // 当选择歌曲时,确定按钮变为可用状态
    connect(songList, &QListWidget::itemSelectionChanged, this, [=](){
        confirmBtn->setEnabled(songList->currentItem() != nullptr);
    });
}

SongLibraryWindow::~SongLibraryWindow()
{
    // 无需delete ui(纯代码创建UI)
}

// 导入歌曲(多种格式)
void SongLibraryWindow::on_importBtn_clicked()
{
    QStringList songPaths = QFileDialog::getOpenFileNames( // 选择文件窗口
        this,
        "选择音乐文件",
        QDir::homePath(),
        "音频文件 (*.mp3 *.wav *.flac *.ogg *.m4a *.aac)"
    );

    if (!songPaths.isEmpty()) { // 判断是否选择文件
        int importedCount = 0; // 导入歌曲总数
        for (const QString &path : songPaths) { // 遍历选择的文件
            QString songName = QFileInfo(path).baseName(); // 提取歌曲名
            bool isDuplicate = false; // 初始化"判断重复"变量
            for (int i = 0; i < songList->count(); i++) { // 检查是否歌名重复
                QListWidgetItem *exsitingItem = songList->item(i);
                if (exsitingItem->text() == songName) {
                    isDuplicate = true;
                    break;
                }
            }
            if (!isDuplicate) { // 如果歌名未重复
                QListWidgetItem *item = new QListWidgetItem(songName); // 创造单个歌曲的列表项目(包含歌曲名称)
                item->setData(Qt::UserRole, path); // 存储歌曲完整路径
                songList->addItem(item); // 将列表项添加至总歌曲列表
                importedCount++;
            }
        }
        if (importedCount > 0){
            QMessageBox::information(this, "成功", "共选择 " + QString::number(songPaths.size()) + "首,成功导入" + QString::number(importedCount) + "首(已跳过重复歌曲)!"); // 窗口提示
        }else {
            QMessageBox::critical(this, "失败", "所选歌曲均已存在，未导入任何新歌曲!");
        }
    }
}

// "确定"选择的一首歌曲后
void SongLibraryWindow::on_confirmBtn_clicked()
{
    QListWidgetItem *selectedItem = songList->currentItem(); // 获取选择的歌曲
    if (!selectedItem) return;

    QList<QUrl> allSongs; // 全部歌曲清单
    int selectedIndex = -1; // 索引初始化
    for (int i = 0; i < songList->count(); i++){ // 全部歌曲清单初始化和寻找欲播放的歌曲索引，并传给主窗口
        QListWidgetItem *item = songList->item(i);
        QString path = item->data(Qt::UserRole).toString();
        allSongs.append(QUrl::fromLocalFile(path));
        if (item == selectedItem) {
            selectedIndex = i;
        }
    }
    emit songSelected(allSongs, selectedIndex);
    this->close();
}