#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QPainter>
#include <QDateTime>
#include <QThread>
#include <QTimer>
#include <QObject>
#include <QImage>
#include <QIcon>
#include <QMouseEvent>
#include <QMessageBox>
#include <QTimer>
#include <QTime>
#include <QPoint>
#include <QRect>
#include <QPixmap>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QPalette>
#include <QRectF>
#include <QDebug>
#include <QQueue>
#include <queue>
#include <QVector>
#include <QString>
#include <QMap>
#include <algorithm>
#include <functional>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QList>

#include "area.h"
#include "pos.h"
#include "zobrist.h"
#include "mythread.h"

#define NONE  0                 // 空位宏
#define BLACK 1                 // 黑子宏
#define WHITE 2                 // 白子宏
#define EXCHANGE 3              // 交替参数宏
#define ADVANTAGE 4             // 棋风参数宏

#define R_INFINTETY  10000

const long long HASH_TABLE_SIZE = 1 << 22;    // 哈希表大小

#define HASH_EXACT 1
#define HASH_ALPHA 2
#define HASH_BETA  3

#define FLAGS_POWER_CONDESE   1
#define FLAGS_POWER_RELEASE   2

//const int chessValue[] = {1,2,2,3,4,5,6,250,600,1000,2000};
const int chessValue[] = {2,8,8,16,32,48,64,250,600,1000,2000};

#define WIDTH_W 1200        // 窗口宽度
#define HEIGH_W 800         // 窗口高度

#define WIDTH_P 640         // 棋盘宽度
#define HEIGH_P 640         // 棋盘高度

#define SIZE 15             // 棋子尺寸

#define N 15                // 横竖线数量

#define Max(a,b) (a)>(b)?(a):(b)
#define Min(a,b) (a)<(b)?(a):(b)
#define FF(a,b,c,d) for(int i=(a); i<(b); i++)for(int j=(c); j<(d); j++)

namespace Ui {
    class MainWindow;
}

class MainWindow :  public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;                         // ui指针
    QThread thread;                             // 线程
    QQueue<Pos> moveQueue;                      // 走法记录
    Pos result;                                 // 接收返回结果
    int centraTimer;                            // 刷新屏幕等待时间
    QMutex mutex;                               // 互斥量，用于操作哈希表
    Area *area;                                 // 棋盘区域
    bool openlog = false;                       // 日志打开标志
    bool runing = false;                        // 运行状态检查，用于搜索
    int limit = 10000, limit_kill=5000;         // 最长运行时间
    bool isdraw[20][20];                        // 绘制表
    int valTab[20][20][3];                      // 估值表
    int priorTab[20][20][3];                    // 权值表
    int val_black=0, val_white=0;               //
    char strTab[3];                             // 字符映射表
    HASHITEM *H[3];                             // 哈希表
    uint64_t Z[20][20][3];                      // 评分置换表
    uint64_t hash = 0;                         // 哈希值
    int chess[20][20];                          // 棋盘数组
    int vis[3][20][20];                         // 棋子能量分布
    int Kernel = 2;                             // 能量分布算子大小
    int order=0;                                // 棋子手顺和最新记录
    int rx[N*N], ry[N*N];                       // 棋子手顺位置
    int rangenum = 30, r=0,c=0;                 // 搜索存储节点数
    QTime t, t2;                                // 计时器
    int depth = 1;
    int guess = 0;
    int cur_x=0, cur_y=0;                       // 鼠标位置
    int px=0, py=0;                             // 鼠标位置对应的棋盘坐标
    int hold=BLACK;                             // 轮走方，默认BLACK
    int algoFlag=2, lock_algo=2;                // 算法使用标志
    bool stop=false;                            // 计算标识，防止错刷屏幕, false为正常刷屏
    bool change=false;                          // 绘图标识，true表示棋盘有逆向变化(悔棋)
    bool painting=false;                        // 连续绘图标识
    QPixmap Pix;                                // 双缓冲绘图
    QPixmap Pix_board;                          // 棋盘图像
    QPainter centerpainter;                     // 中心绘图设备，与双缓冲绘图匹配
    QString buffer;                             // 屏幕信息

    const int delta = 2;
    const int init_depth = 2;
    const int vx[8] = { 0, 1, 1, 1, 0,-1,-1,-1};
    const int vy[8] = {-1,-1, 0, 1, 1, 1, 0,-1};
    const int add[15][15] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0},
        {0, 0, 2, 3, 4, 4, 4, 4, 4, 4, 4, 3, 2, 0, 0},
        {0, 0, 2, 4, 5, 5, 5, 5, 5, 5, 5, 4, 2, 0, 0},
        {0, 0, 2, 4, 5, 5, 5, 5, 5, 5, 5, 4, 2, 0, 0},
        {0, 0, 2, 4, 5, 5, 6, 6, 6, 5, 5, 4, 2, 0, 0},
        {0, 0, 2, 4, 5, 5, 6, 7, 6, 5, 5, 4, 2, 0, 0},
        {0, 0, 2, 4, 5, 5, 6, 6, 6, 5, 5, 4, 2, 0, 0},
        {0, 0, 2, 4, 5, 5, 5, 5, 5, 5, 5, 4, 2, 0, 0},
        {0, 0, 2, 4, 5, 5, 5, 5, 5, 5, 5, 4, 2, 0, 0},
        {0, 0, 2, 3, 4, 4, 4, 4, 4, 4, 4, 3, 2, 0, 0},
        {0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    };

protected:
    // 鼠标响应事件函数
    void mousePressEvent(QMouseEvent *);
    // 计时器响应时间函数
    void timerEvent(QTimerEvent *);
    // 绘图响应函数
    void paintEvent(QPaintEvent *);
    // 绘制棋子函数
    void drawAll();
    void drawChess(int px,          // 待绘制棋子x轴坐标
                   int py,          // 待绘制棋子y轴坐标
                   QColor color);    // 绘制颜色
    // 子力评估函数
    int valueChess(int x, int y, int key, int *piority);
    int evaluate(int key);

    // 根据策略获取落子位置
    void getPosition(int &x,int &y, int key, int flag);
    void powerOperation(int x, int y, int flag, int key);
    void callFunction(Pos& newMove, int flag, const int& judge);
    bool distribution(int key, int time);

    // 胜负判断
    int checkWinner(int x, int y, bool endFlag);

    // 越界判断函数
    bool inline inside(int x, int y);
    bool inline inside(Pos move);

    // 调试使用函数
    void showChess();

    // 置换表操作
    bool lookup(int depth, int alpha, int beta, int &val, int flag);
    bool store(QMutex &m, int depth, int val, int hashf, long long hashIndex, int flag);
    unsigned long long rand64();

// 槽函数，用于控件通信
private slots:
    // 上一手功能实现
    void on_action_Back_triggered();
    // 电脑走棋实现
    void on_action_Help_triggered();
    // 以下均用于调整参数
    void on_radioButton_clicked();
    void on_radioButton_2_clicked();
    void on_radioButton_9_clicked();
    void on_action_Version_triggered();
    void on_pushButton_clicked();
    void on_lineEdit_textChanged(const QString &arg1);
    void on_lineEdit_2_textChanged(const QString &arg1);

public slots:
    void dealSignal(const QString &str);
signals:
    void startThread(const QString &str);
};

template <class T> struct greater {
    bool operator() (const T& x, const T& y) const {return x>y;}
};

template <class T> struct less {
    bool operator() (const T& x, const T& y) const {return x<y;}
};

template<class T>
void _swap(T &a, T &b){
    T tmp=a;a=b;b=tmp;
}

#endif // MAINWINDOW_H
