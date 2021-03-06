#include "mythread.h"
#include <QThread>
#include <QDebug>
#include <QMessageBox>

extern QVector<Pos> root;
extern int Count, ABcut, tag, sto;

MyThread::MyThread(QObject *parent) : QObject(parent)
{
    isStop = false;
}

void MyThread::initial(HASHITEM *_H, uint64_t _Z[20][20][3], uint64_t _hash,
                        int _chess[20][20], int _vis[3][20][20], int key, int _limit,
                        int _depth, int _algoFlag, bool _openlog, int _order)
{
    hash = _hash;
    hold = key;
    for(int i = 0; i < 15; i++){
        for(int j = 0; j < 15; j++){
            chess[i][j] = _chess[i][j];
            vis[1][i][j] = _vis[1][i][j];
            vis[2][i][j] = _vis[2][i][j];
            Z[i][j][1] = _Z[i][j][1];
            Z[i][j][2] = _Z[i][j][2];
        }
    }
    limit = _limit;
    depth = _depth;
    algoFlag = _algoFlag;
    openlog = _openlog;
    order = _order;
    H = new HASHITEM[HASH_TABLE_SIZE]();
    if(H == nullptr){
        qDebug("failed to create hashtable");
    }
}

MyThread::~MyThread(){
     delete H;
}

void MyThread::dowork(const QString& str)
{
    Pos ret(20, 20);
    QVector<Pos> path;
    if(isStop == false){
        t2.start();
        if(limit > 0 && depth < 30)
        {

            topFlag = true;
            if(algoFlag == 1){
                deepSearch(ret, hold, hold, depth, depth, -R_INFINTETY, R_INFINTETY, path);
            }else if(algoFlag == 0){
                PVS(ret, hold, depth, -R_INFINTETY,R_INFINTETY, path);
            }else if(algoFlag == 2){
                int guess =str.toLong();
                MTD(ret, hold, guess, depth);
            }
        }
        QString temp;
        if(algoFlag == 2){
            temp.sprintf("%d,%d,%d,%d,%d,",5 ,ret.x, ret.y, ret.value, root.size());
        }else{
            temp.sprintf("%d,%d,%d,%d,%d,",5 ,ret.x, ret.y, ret.value, 10);
        }
//        qDebug("target=%d, store=%d", tag, sto);

        if(runing && limit > 0) temp += "1";
        else temp += "0";
        emit resultReady(temp);
        isStop = true;
    }
    else{
        QThread::sleep(1);
    }
}

void MyThread::setFlag(bool flag){
    isStop = flag;
}

int MyThread::valueChess(int x, int y, int key, int *piority){
    int i, j;

    int p[8];                   // p对应方向的子力
    int b[8];                   // blank对应方向的子力，之后的空位
    int bp[8];                  // bp对应方向的子力，之后的空位，之后的子力
    int bpb[8];                 // bpb对应方向的子力，之后的空位，之后的子力，之后的空位
    int two=0, three=0, jump=0, four=0, five=0;
    int sleep_three=0, sleep_two=0, sleep_jump=0;

    memset(p, 0, sizeof(p));
    memset(b, 0, sizeof(b));
    memset(bp, 0, sizeof(bp));
    memset(bpb, 0, sizeof(bpb));

    // 方向从上方开始，顺时针寻找
    for (i = 0, j = 1; i < 8; i++, j = 1){
        for(; j <= 5 && inside(x+vx[i]*j, y+vy[i]*j) && chess[x+vx[i]*j][y+vy[i]*j] == key; j++, p[i]++);
        for(; j <= 5 && inside(x+vx[i]*j, y+vy[i]*j) && chess[x+vx[i]*j][y+vy[i]*j] == 0;   j++, b[i]++);
        for(; j <= 5 && inside(x+vx[i]*j, y+vy[i]*j) && chess[x+vx[i]*j][y+vy[i]*j] == key; j++, bp[i]++);
        for(; j <= 5 && inside(x+vx[i]*j, y+vy[i]*j) && chess[x+vx[i]*j][y+vy[i]*j] == 0;   j++, bpb[i]++);
    }

    for (i = 0; i < 4; i++){

        if(p[i] + p[i+4] >= 4) {
            // OOOOO
            five++;
        }

        if(p[i] + p[i+4] == 3){
            // +OOOO+
            if(b[i] >= 1 && b[i+4] >= 1 )       { four += 2;}      // 四连
            // +OOOO
            if(b[i] == 0 && b[i+4] >= 1 )       { four++;}      // 冲四
            if(b[i] >= 1 && b[i+4] == 0 )       { four++;}      // 冲四
        }

        if(p[i] + p[i+4] == 2){
            // OOO+O
            if(b[i]   == 1 && bp[i]   >= 1 )    { four++;} // 跳四
            if(b[i+4] == 1 && bp[i+4] >= 1 )    { four++;}
            // ++OOO+
            if     (b[i] >= 2 && b[i+4] >= 1 && b[i] + b[i+4] >= 3 && bp[i+4] == 0)   { three++;}   // 活三
            else if(b[i] >= 1 && b[i+4] >= 2 && b[i] + b[i+4] >= 3 && bp[i]   == 0)   { three++;}   // 活三
            // OOO++ // +OOO+
            if(b[i] == 1 && b[i+4] == 1)           { sleep_three++;}    // 眠三
            else if(b[i] == 0 && b[i+4] >= 2)      { sleep_three++;}    // 眠三
            else if(b[i+4] == 0 && b[i] >= 2)      { sleep_three++;}    // 眠三
        }

        if(p[i] + p[i+4] == 1){
            // OO+OO
            if(b[i]   == 1 && bp[i]   >= 2 )    { four++;}   // 跳四
            if(b[i+4] == 1 && bp[i+4] >= 2 )    { four++;}
            // +OO+O+
            if     (b[i]   == 1 && bp[i]   == 1 && bpb[i] >= 1 && b[i+4] >= 1 && b[i+4] - bp[i+4] > 0)   { jump++;} // 跳三
            else if(b[i+4] == 1 && bp[i+4] == 1 && b[i] >= 1 && bpb[i+4] >= 1 && b[i] - bp[i] > 0)   { jump++;} // 跳三

            // OO+O+ or +OO+O
            if     (b[i]   == 1 && bp[i]   == 1 && bpb[i] + b[i+4] == 1 )   { sleep_jump++;}       // 眠三
            else if(b[i+4] == 1 && bp[i+4] == 1 && b[i] + bpb[i+4] == 1 )   { sleep_jump++;}
            // OO++O
            if     (b[i]   == 2 && bp[i]   >= 1 )                   { sleep_jump++;}
            else if(b[i+4] == 2 && bp[i+4] >= 1 )                   { sleep_jump++;}
            // +++OO++ && ++OO+++
            if (b[i] >= 1 && b[i+4] >= 1 && b[i] + b[i+4] >= 5)  { two++; }       // 活二
            else if (b[i] + b[i+4] <= 5)  { sleep_two++; }       // 眠二
            else if (b[i] == 0 && b[i+4] >= 5)  { sleep_two++; }       // 眠二
            else if (b[i+4] == 0 && b[i] >= 5)  { sleep_two++; }       // 眠二

        }

        if(p[i] + p[i+4] == 0){
            // O+OOO
            if(b[i]   == 1 && bp[i]   >= 3 )    { four++;}
            if(b[i+4] == 1 && bp[i+4] >= 3 )    { four++;}
            // +O+OO+
            if     (b[i]   == 1 && bp[i]   == 2 && bpb[i]   >= 1 && b[i+4] >= 1)   { jump++;}
            else if(b[i+4] == 1 && bp[i+4] == 2 && bpb[i+4] >= 1 && b[i]   >= 1)   { jump++;}

            // O+OO+ && +O+OO
            if((b[i] == 1 && bp[i] == 2 && (bpb[i] >= 1 ||  b[i+4] >= 1)))           { sleep_jump++;}
            else if((b[i+4] == 1 && bp[i+4] == 2 && (bpb[i+4] >= 1 ||  b[i] >= 1)))  { sleep_jump++;}

            // +O+O++
            if(b[i]   >= 2 && b[i+4] == 1 && bp[i+4] == 1 &&  bpb[i+4] >= 1)    { two++;}
            if(b[i+4] >= 2 && b[i]   == 1 && bp[i]   == 1 &&  bpb[i]   >= 1)    { two++;}

            // +O++O+
            if(b[i]   >= 1 && b[i+4] == 2 && bp[i+4] == 1 &&  bpb[i+4] >= 1)    { two++;}
            if(b[i+4] >= 1 && b[i]   == 2 && bp[i]   == 1 &&  bpb[i]   >= 1)    { two++;}

            // O+O++ or ++O+O
            if(b[i]   >= 2 && b[i+4] == 1 && bp[i+4] == 1 &&  bpb[i+4] == 0)    { sleep_two++;}
            if(b[i+4] >= 2 && b[i]   == 1 && bp[i]   == 1 &&  bpb[i]   == 1)    { sleep_two++;}
            if(b[i]   == 0 && b[i+4] == 1 && bp[i+4] == 1 &&  bpb[i+4] >= 2)    { sleep_two++;}
            if(b[i+4] == 0 && b[i]   == 1 && bp[i]   == 1 &&  bpb[i]   >= 2)    { sleep_two++;}
        }
    }

     *piority = jump + three + 100*four + 10000*five;

    if (five >= 1)
        return chessValue[10];

    if (four >= 2)
        return chessValue[9];

    if (four >= 1 && jump+three >= 1)
        return chessValue[8];

    if (jump+three >= 2)
        return chessValue[7];

    return Max(chessValue[0]*sleep_two + chessValue[1]*sleep_three +
           chessValue[2]*sleep_jump + chessValue[3]*two + chessValue[4]*jump +
           chessValue[5]*three + chessValue[6]*four, add[x][y]);
}

bool inline MyThread::inside(int x, int y){
    if (x < 15 && y < 15 && x >= 0 && y >= 0)
        return true;
    return false;
}

bool inline MyThread::inside(Pos move){
    if (move.x < 15 && move.y < 15 && move.x >= 0 && move.y >= 0)
        return true;
    return false;
}

int MyThread::deepSearch(Pos& ret, int origin, int key, int deep, int rec, int alpha, int beta, QVector<Pos>& path)
{
    int i, j, p1, p2, k, dx, dy;
    Pos newMove;
    QVector<Pos> attackQueue, vec_moves;

    if(t2.elapsed() > limit){
        if(runing) runing = false;
        return alpha;
    }

    // 检查游戏是否结束
    if(rec > deep && path.last().a1 >= 10000)
        return -R_INFINTETY;

    if(lookup(deep, alpha, beta, newMove)){
        return newMove.value;
    }

    // 获取着法
    for (i = 0; i < 15; i++){
        for (j = 0; j < 15; j++){
            if((vis[2][i][j] >= 2 || (vis[2][i][j] >= 1 && depth==deep)) && chess[i][j] == 0){
                // 算杀的结点选择
                // 进攻方：活三、冲四、活四、五连、防守对方的冲四
                // 防守方：防守对方的活三、冲四，自身的冲四
                k = Max(1.5*valueChess(i, j, key, &p1), valueChess(i, j, 3-key, &p2));
                if(origin == key){
                    // 进攻方选点，己方的进攻棋，和对方的冲四棋
                    if(p1 > 0 || p2 >= 10000){
                        attackQueue.push_back(Pos(i, j, k, p1, rec-deep, p2));
                    }
                }else{
                    // 防守方选点，己方的冲四棋，和对方的进攻棋
                    if(p2 > 0 || p1 >= 100){
                        attackQueue.push_back(Pos(i, j, k, p1, rec-deep, p2));
                    }
                }
            }
        }
    }

    // 进攻方无棋
    if(origin == key && attackQueue.isEmpty())
        return alpha;
    // 防守方无棋
    if(origin == 3-key && attackQueue.isEmpty()){
        // 考虑进攻方的进攻导致防守方无棋
        if(rec > deep && path.last().a1 == 10000)
            return -R_INFINTETY;
        // 考虑防守方的反击导致防守方无棋
        if(rec > deep && path.last().a3 == 10000)
            return R_INFINTETY;
        return -R_INFINTETY;
    }

    Count += attackQueue.size();
    qSort(attackQueue.begin(), attackQueue.end(), [](Pos& a, Pos &b){
        return a.value > b.value;
    });

    //五连棋型
    if (attackQueue[0].value >= 2000){
        for(i = 0;i < attackQueue.size() && attackQueue[i].value >= 2000; i++)
            vec_moves.push_back(attackQueue[i]);
    }
    //己方四连
    else if (attackQueue[0].value >= 1500){

        for(i = 0; i < attackQueue.size() && attackQueue[i].value >= 1500; i++)
            vec_moves.push_back(attackQueue[i]);

    }
    //对方四连
    else if (attackQueue[0].value >= 1000){

        // 收集对方活四
        for(i = 0; i < attackQueue.size() && attackQueue[i].value >= 1000; i++){
            vec_moves.push_back(attackQueue[i]);
        }

        // 收集己方冲四和延伸冲四
        for(; i < attackQueue.size(); i++){
            if(attackQueue[i].a1 >= 100)
                vec_moves.push_back(attackQueue[i]);
            else if(attackQueue[i].a3 >= 100){
                dx = abs(attackQueue[i].x - attackQueue[0].x);
                dy = abs(attackQueue[i].y - attackQueue[0].y);
                if((dx == dy && dx <= 4) || (dx == 0 && dy <= 4) || (dy == 0 && dx <= 4))
                    vec_moves.push_back(attackQueue[i]);
            }
        }

    }else{
        vec_moves = attackQueue;
    }

    ABcut += attackQueue.size() - vec_moves.size();

    // 如果进攻方没找到结点，情况如下：
    // 1.局面没有进攻结点了，表明进攻失败，应该返回alpha。
    // 2.进攻过程中遭到反击，如反活三，说明进攻不能成功，应该返回alpha。
    // 如果防守方没找到结点，情况如下：
    // 1.无法阻挡攻势，应该返回-INF。

    // 进攻方无棋
    if(origin == key && vec_moves.isEmpty())
        return alpha;
    // 防守方无棋
    if(origin == 3-key && vec_moves.isEmpty()){
        // 考虑进攻方的进攻导致防守方无棋
        if(rec > deep && path.last().a1 == 10000)
            return -R_INFINTETY;
        // 考虑防守方的反击导致防守方无棋
        if(rec > deep && path.last().a3 == 10000)
            return R_INFINTETY;
        return -R_INFINTETY;
    }

    for(Pos& move: vec_moves){

        powerOperation(move.x, move.y, FLAGS_POWER_CONDESE, key);
        path.push_back(move);

        if(deep > 1)
            move.value = - deepSearch(ret, origin, 3-key, deep-1, rec, -beta, -alpha, path);
        else
            move.value = evaluate(key);

//        QString tmp, out;
//        for(auto &a: path){
//            tmp.sprintf("[%d,%d],", a.x, a.y);
//            out += tmp;
//        }
//        tmp.sprintf("=%d",move.value);
//        out += tmp;
//        qDebug(out.toLatin1());

        path.pop_back();
        powerOperation(move.x, move.y, FLAGS_POWER_RELEASE, key);

        if(move.value > alpha){
            alpha = move.value;
            update(mutex, ret, move);
        }

        if (move.value >= beta){
            ABcut++;
            return beta;
        }

    }
    return alpha;
}

int MyThread::PVS(Pos& ret, int key, int deep, int alpha, int beta, QVector<Pos>& path)
{
    int i, j, p1, p2, k, dx, dy;
    int hashf = HASH_ALPHA;
    uint64_t hashIndex=0, hashBest;
    Pos newMove, bestMove;
    QVector<Pos> attackQueue, vec_moves, tmp;

    if(t2.elapsed() > limit){
        if(runing) runing = false;
        return alpha;
    }

    if(depth > deep && path.last().a1 >= 10000)
        return -R_INFINTETY;

    if(lookup(deep, alpha, beta, newMove)){

        newMove.a2 = depth - deep;
        valueChess(newMove.x, newMove.y, key, &newMove.a1);

        update(mutex, ret, newMove);
        return newMove.value;
    }

    if(depth == deep && !root.empty()){

        // 读取记录
        vec_moves = root;

    }else{

        // 生成合适着法
        for (i = 0; i < 15; i++){
            for (j = 0; j < 15; j++){
                if (vis[2][i][j] >= 2 && chess[i][j] == 0){
                    k = Max(1.5*valueChess(i, j, key, &p1), valueChess(i, j, 3-key, &p2));
                    attackQueue.push_back(Pos(i, j, k, p1, depth-deep, p2));
                }
            }
        }

        Count += (attackQueue.size() > rangenum) ? rangenum : attackQueue.size();
        qSort(attackQueue.begin(), attackQueue.end(), [](Pos& a, Pos &b){
            return a.value > b.value;
        });

        //五连棋型
        if (attackQueue[0].value >= 2000){

            for(i = 0;i < attackQueue.size() && attackQueue[i].value >= 2000; i++)
                vec_moves.push_back(attackQueue[i]);

            if(depth == deep && root.empty()){
                root = vec_moves;
            }

        }
        //己方四连
        else if (attackQueue[0].value >= 1500){

            for(i = 0;i < attackQueue.size() && attackQueue[i].value >= 1500; i++)
                vec_moves.push_back(attackQueue[i]);

            if(depth == deep && root.empty()){
                root = vec_moves;
            }
        }
        //对方四连
        else if (attackQueue[0].value >= 1000){

            if(depth == deep && root.empty()){

                // 首发剪枝
                for(i = 0; i < attackQueue.size() && attackQueue[i].value >= 1000; i++)
                    tmp.push_back(attackQueue[i]);

                for(; i < attackQueue.size(); i++){
                    if(attackQueue[i].a1 >= 100)
                        tmp.push_back(attackQueue[i]);
                    else if(attackQueue[i].a3 >= 100){
                        dx = abs(attackQueue[i].x - attackQueue[0].x);
                        dy = abs(attackQueue[i].y - attackQueue[0].y);
                        if((dx == dy && dx <= 4) || (dx == 0 && dy <= 4) || (dy == 0 && dx <= 4))
                            tmp.push_back(attackQueue[i]);
                    }
                }

                cutTreeNode(tmp, vec_moves, path, key);
                root = vec_moves;

            }else{

                for(i = 0; i < attackQueue.size() && attackQueue[i].value >= 1000; i++)
                    vec_moves.push_back(attackQueue[i]);

                for(; i < attackQueue.size(); i++){
                    if(attackQueue[i].a1 >= 100)
                        vec_moves.push_back(attackQueue[i]);
                    else if(attackQueue[i].a3 >= 100){
                        dx = abs(attackQueue[i].x - attackQueue[0].x);
                        dy = abs(attackQueue[i].y - attackQueue[0].y);
                        if((dx == dy && dx <= 4) || (dx == 0 && dy <= 4) || (dy == 0 && dx <= 4))
                            vec_moves.push_back(attackQueue[i]);
                    }
                }

            }

        }else{
            if(depth == deep && root.empty()){
                // 首发剪枝
                cutTreeNode(attackQueue, vec_moves, path, key);
                root = attackQueue;
            }else{
                // 普通情况
                vec_moves = attackQueue;
            }
        }

        ABcut += attackQueue.size() - vec_moves.size();
    }

    int cnt=0, cur = -R_INFINTETY;
    for(Pos& move: vec_moves){

        if(cnt++ > rangenum){
            break;
        }

        hashIndex = hash;
        powerOperation(move.x, move.y, FLAGS_POWER_CONDESE, key);
        path.push_back(move);

        if(deep > 1){
            if(move == vec_moves[0]){
                move.value = - PVS(ret, EXCHANGE - key, deep - 1, -beta, -alpha, path);
            }
            else{
                move.value = - PVS(ret, EXCHANGE - key, deep - 1, -alpha-1, -alpha, path);
                if(alpha < move.value && move.value < beta){
                    move.value = - PVS(ret, EXCHANGE - key, deep - 1, -beta, -alpha, path);
                }
            }
        }else{
            move.value = evaluate(key);
            store(mutex, HASH_EXACT, hash, move, deep);
        }

        path.pop_back();
        powerOperation(move.x, move.y, FLAGS_POWER_RELEASE, key);

        if(cur < move.value){
            cur = move.value;

            if(alpha < move.value){
                alpha = move.value;
                hashf = HASH_EXACT;
                hashBest = hashIndex;
                bestMove = move;
                update(mutex, ret, move);

            }
            if(beta <= move.value){
                ABcut++;
                if(runing) store(mutex, HASH_BETA, hashIndex, move, deep);
                break;
            }
        }
    }
    if(runing) store(mutex, hashf, hashBest, bestMove, deep);
    return cur;
}

int MyThread::checkSearch(Pos& ret, int origin, int key, int deep, int rec, int alpha, int beta, QVector<Pos>& path)
{
    int i, j, p1, p2, k, dx, dy;
    Pos newMove;
    QVector<Pos> attackQueue, vec_moves;

    if(t2.elapsed() > limit/2){
        if(runing) runing = false;
        return alpha;
    }

    // 检查游戏是否结束
    if(rec > deep && path.last().a1 >= 10000)
        return -R_INFINTETY;

    if(lookup(deep, alpha, beta, newMove)){
        return newMove.value;
    }

    // 获取着法
    for (i = 0; i < 15; i++){
        for (j = 0; j < 15; j++){
            if((vis[2][i][j] >= 2 || (vis[2][i][j] >= 1 && depth==deep)) && chess[i][j] == 0){
                // 算杀的结点选择
                // 进攻方：活三、冲四、活四、五连、防守对方的冲四
                // 防守方：防守对方的活三、冲四，自身的冲四
                k = Max(1.5*valueChess(i, j, key, &p1), valueChess(i, j, 3-key, &p2));
                if(origin == key){
                    // 进攻方选点，己方的进攻棋，和对方的冲四棋
                    if(p1 > 0 || p2 >= 10000){
                        attackQueue.push_back(Pos(i, j, k, p1, rec-deep, p2));
                    }
                }else{
                    // 防守方选点，己方的冲四棋，和对方的进攻棋
                    if(p2 > 0 || p1 >= 100){
                        attackQueue.push_back(Pos(i, j, k, p1, rec-deep, p2));
                    }
                }
            }
        }
    }

    // 进攻方无棋
    if(origin == key && attackQueue.isEmpty())
        return alpha;
    // 防守方无棋
    if(origin == 3-key && attackQueue.isEmpty()){
        // 考虑进攻方的进攻导致防守方无棋
        if(rec > deep && path.last().a1 == 10000)
            return -R_INFINTETY;
        // 考虑防守方的反击导致防守方无棋
        if(rec > deep && path.last().a3 == 10000)
            return R_INFINTETY;
        return -R_INFINTETY;
    }

    Count += attackQueue.size();
    qSort(attackQueue.begin(), attackQueue.end(), [](Pos& a, Pos &b){
        return a.value > b.value;
    });

    //五连棋型
    if (attackQueue[0].value >= 2000){

        for(i = 0; i < attackQueue.size() && attackQueue[i].value >= 2000; i++)
            vec_moves.push_back(attackQueue[0]);
    }
    //己方四连
    else if (attackQueue[0].value >= 1500){

        for(i = 0; i < attackQueue.size() && attackQueue[i].value >= 1500; i++)
            vec_moves.push_back(attackQueue[i]);
    }
    //对方四连
    else if (attackQueue[0].value >= 1000){

        // 收集对方活四

        for(i = 0; i < attackQueue.size() && attackQueue[i].value >= 1000; i++){
            vec_moves.push_back(attackQueue[i]);
        }

        // 收集己方冲四和延伸冲四

        for(; i < attackQueue.size(); i++){
            if(attackQueue[i].a1 >= 100)
                vec_moves.push_back(attackQueue[i]);
            else if(attackQueue[i].a3 >= 100){
                dx = abs(attackQueue[i].x - attackQueue[0].x);
                dy = abs(attackQueue[i].y - attackQueue[0].y);
                if((dx == dy && dx <= 4) || (dx == 0 && dy <= 4) || (dy == 0 && dx <= 4))
                    vec_moves.push_back(attackQueue[i]);
            }
        }

    }else{
        vec_moves = attackQueue;
    }

    ABcut += attackQueue.size() - vec_moves.size();

    // 如果进攻方没找到结点，情况如下：
    // 1.局面没有进攻结点了，表明进攻失败，应该返回alpha。
    // 2.进攻过程中遭到反击，如反活三，说明进攻不能成功，应该返回alpha。
    // 如果防守方没找到结点，情况如下：
    // 1.无法阻挡攻势，应该返回-INF。

    // 进攻方无棋
    if(origin == key && vec_moves.isEmpty())
        return alpha;
    // 防守方无棋
    if(origin == 3-key && vec_moves.isEmpty()){
        // 考虑进攻方的进攻导致防守方无棋
        if(rec > deep && path.last().a1 == 10000)
            return -R_INFINTETY;
        // 考虑防守方的反击导致防守方无棋
        if(rec > deep && path.last().a3 == 10000)
            return R_INFINTETY;
        return -R_INFINTETY;
    }

    for(Pos& move: vec_moves){

        powerOperation(move.x, move.y, FLAGS_POWER_CONDESE, key);
        path.push_back(move);

        if(deep > 1)
            move.value = - checkSearch(ret, origin, 3-key, deep-1, rec, -beta, -alpha, path);
        else{
            move.value = evaluate(key);
            store(mutex, HASH_EXACT, hash, move, 1);
        }

        path.pop_back();
        powerOperation(move.x, move.y, FLAGS_POWER_RELEASE, key);

        if(move.value > alpha){
            alpha = move.value;
            update(mutex, ret, move);
        }

        if (move.value >= beta){
            ABcut++;
            return beta;
        }

    }
    return alpha;

}

void MyThread::MTD(Pos& bestmove, int origin, int f, int deep)
{
    int best_value=f, test, bound[2]={-10000, 10000};
    Pos newMove;
    QVector<Pos> path;

    do{
        test = best_value + (best_value == bound[0]);
        best_value = MT(newMove, origin, deep, test-1, test, path);
        bound[best_value < test] = best_value;
        if(best_value >= test && runing){
            bestmove = newMove;
            bestmove.value = bound[0];
        }
    }while(bound[0] < bound[1]);
}

int MyThread::MT(Pos& ret, int key, int deep, int alpha, int beta, QVector<Pos>& path)
{
    int i, j, p1, p2, cur=-R_INFINTETY, k, dx, dy;
    int hashf = HASH_ALPHA;
    long long hashIndex=0, hashBest;
    Pos newMove, bestMove;
    QVector<Pos> vec_moves, attackQueue, tmp;

    if(t2.elapsed() > limit){
        if(runing) runing = false;
        return alpha;
    }

    // 检查游戏是否结束
    if(depth > deep && path.last().a1 >= 10000){
        return -R_INFINTETY;
    }

    // 查找哈希表
    if(lookup(deep, alpha, beta, newMove)){

        newMove.a2 = depth - deep;
        valueChess(newMove.x, newMove.y, key, &newMove.a1);

        update(mutex, ret, newMove);
        return newMove.value;
    }

    if(depth == deep && !root.empty()){

        // 读取记录
        vec_moves = root;

    }else{

        // 生成合适着法
        FF(0, 15, 0, 15){
            if (vis[2][i][j] >= 2 && chess[i][j] == 0){
                k = Max(1.5*valueChess(i, j, key, &p1), valueChess(i, j, 3-key, &p2));
                attackQueue.push_back(Pos(i, j, k, p1, depth-deep, p2));
            }
        }

        Count += (attackQueue.size() > rangenum) ? rangenum : attackQueue.size();

        qSort(attackQueue.begin(), attackQueue.end(), [](Pos& a, Pos &b){
            return a.value > b.value;
        });

        //五连棋型
        if (attackQueue[0].value >= chessValue[10]){

            for(i = 0;i < attackQueue.size() && attackQueue[i].value >= chessValue[10]; i++)
                vec_moves.push_back(attackQueue[i]);

            if(depth == deep && root.empty()){
                root = vec_moves;
            }

        }
        //己方四连
        else if (attackQueue[0].value >= 1.5*chessValue[9]){

            for(i = 0; i < attackQueue.size() && attackQueue[i].value >= 1.5*chessValue[9]; i++)
                vec_moves.push_back(attackQueue[i]);

            if(depth == deep && root.empty()){
                root = vec_moves;
            }
        }
        //对方四连
        else if (attackQueue[0].value >= chessValue[9]){

            if(depth == deep && root.empty()){

                // 首发剪枝
                for(i = 0; i < attackQueue.size() && attackQueue[i].value >= chessValue[9]; i++)
                    tmp.push_back(attackQueue[i]);

                for(; i < attackQueue.size(); i++){
                    if(attackQueue[i].a1 >= 100)
                        tmp.push_back(attackQueue[i]);
                    else if(attackQueue[i].a3 >= 100){
                        dx = abs(attackQueue[i].x - attackQueue[0].x);
                        dy = abs(attackQueue[i].y - attackQueue[0].y);
                        if((dx == dy && dx <= 4) || (dx == 0 && dy <= 4) || (dy == 0 && dx <= 4))
                            tmp.push_back(attackQueue[i]);
                    }
                }

                cutTreeNode(tmp, vec_moves, path, key);
                root = vec_moves;

            }else{

                for(i = 0; i < attackQueue.size() && attackQueue[i].value >= chessValue[9]; i++)
                    vec_moves.push_back(attackQueue[i]);

                for(; i < attackQueue.size(); i++){
                    if(attackQueue[i].a1 >= 100)
                        vec_moves.push_back(attackQueue[i]);
                    else if(attackQueue[i].a3 >= 100){
                        dx = abs(attackQueue[i].x - attackQueue[0].x);
                        dy = abs(attackQueue[i].y - attackQueue[0].y);
                        if((dx == dy && dx <= 4) || (dx == 0 && dy <= 4) || (dy == 0 && dx <= 4))
                            vec_moves.push_back(attackQueue[i]);
                    }
                }

            }

        }else{
            if(depth == deep && root.empty()){
                // 首发剪枝
                cutTreeNode(attackQueue, vec_moves, path, key);
                root = attackQueue;
            }else{
                // 普通情况
                vec_moves = attackQueue;
            }
        }
        ABcut += attackQueue.size() - vec_moves.size();
    }

    // 遍历搜索树
    int cnt = 0;

    for(Pos& move: vec_moves){

        if(cnt++ > rangenum){
            break;
        }

        hashIndex = hash;
        powerOperation(move.x, move.y, FLAGS_POWER_CONDESE, key);
        path.push_back(move);

        if(deep > 1)
            move.value = - MT(ret, 3-key, deep-1, -beta, -alpha, path);
        else{
            move.value = evaluate(key);
            if(runing) store(mutex, HASH_EXACT, hash, move, deep);
        }

        powerOperation(move.x, move.y, FLAGS_POWER_RELEASE, key);
        path.pop_back();

        if(move.value > cur){
            cur = move.value;

            // 更新最优解
            if(move.value > alpha){
                alpha = move.value;
                hashf = HASH_EXACT;
                hashBest = hashIndex;
                bestMove = move;
                update(mutex, ret, move);
            }

            // 剪枝
            if(move.value >= beta){
                ABcut++;
                if(runing) store(mutex, HASH_BETA, hashIndex, move, deep);
                break;
            }
        }
    }
    if(runing) store(mutex, hashf, hashBest, bestMove, deep);
    return cur;
}

void MyThread::cutTreeNode(QVector<Pos>& queue_move, QVector<Pos>& vec_moves, QVector<Pos>& path, int key){

    if(queue_move.isEmpty())
        return;

    Pos newMove;
    int MaxDeep=30, val, pri[60]={0}, i;

    for(int d = 4; d <= MaxDeep; d += 4){

        i = 0;
        for(Pos &move: queue_move){

            if(pri[i] > 0) continue;

            powerOperation(move.x, move.y, FLAGS_POWER_CONDESE, key);
            path.push_back(move);

            val = checkSearch(newMove, 3-key, 3-key, d, d, -R_INFINTETY, R_INFINTETY, path);

            path.pop_back();
            powerOperation(move.x, move.y, FLAGS_POWER_RELEASE, key);

            if(runing){
                if(val >= chessValue[10]){
                   pri[i] = d;
                   if(openlog) qDebug("[%d,%d]=%d", move.x, move.y, d);
                }
            }

            i++;
        }
    }

    for(i = 0; i < queue_move.size(); i++)
        if(pri[i] == 0)
            vec_moves.push_back(queue_move[i]);

    // 当全为必败着法时，保存最长的结果
    if(vec_moves.isEmpty()){

        if(openlog){
            qDebug("No enough move, find longest move");
        }

        qSort(queue_move.begin(), queue_move.end(), [](Pos &a, Pos &b){
            return a.a3 > b.a3;
        });

        vec_moves.push_back(queue_move[0]);

    }

    if(openlog){
        qDebug("[%d -> %d]", queue_move.size(), vec_moves.size());
    }

    if(!runing) runing = true;
}

void MyThread::update(QMutex& m, Pos& ret, const Pos ref)
{
    QMutexLocker locker(&m);
    if(inside(ref) && ref.a2 == ret.a2 && (ref.value > ret.value || !inside(ret))){
//        if(openlog) qDebug("(%d,%d,%d)->(%d,%d,%d) alpha", ret.x, ret.y, ret.value, ref.x, ref.y, ref.value);
        ret.x = ref.x;
        ret.y = ref.y;
        ret.value = ref.value;
    }
}

int MyThread::evaluate(int key)
{
    int o_prior=0, d_prior=0, o_val=0, d_val=0, p;

    FF(0, N, 0, N){
        if(chess[i][j] == 3-key)
        {
            o_val += (o_val, valueChess(i, j, 3-key, &p));
            o_prior =  Max(o_prior, p);

        }
        if(chess[i][j] == key){
            d_val += (d_val,valueChess(i, j, key, &p));
            d_prior = Max(d_prior, p);
        }
    }
    // 后手方五连
    if (d_prior >= 10000)
        return R_INFINTETY;
    // 先手方五连
    if (o_prior >= 10000)
        return -R_INFINTETY;
    // 先手方有先手，后手方无更高级先手
    if(o_prior > 0 && d_prior < 100)
        return -R_INFINTETY;
    // 冲四
    if(o_prior >= 100 && d_prior < 10000)
        return -R_INFINTETY;
    // 五连
    if(o_prior >= 10000 && d_prior >= 10000)
        return -R_INFINTETY;

    return d_val - o_val;
}

void MyThread::powerOperation(int x, int y, int flag, int key)
{
    int i, j, dx, dy, k, p;

    if (flag == FLAGS_POWER_CONDESE){
        order++;
        hash ^= Z[x][y][key];
        chess[x][y] = key;
        FF(0, 8, 1, 3){
            dx = x + vx[i]*j;
            dy = y + vy[i]*j;
            if(inside(dx, dy)){
                vis[2][dx][dy]++;
            }
        }
    }
    else{
        order--;
        hash ^= Z[x][y][key];
        chess[x][y] = 0;
        FF(0, 8, 1, 3){
            dx = x + vx[i]*j;
            dy = y + vy[i]*j;
            if(inside(dx, dy)){
                vis[2][dx][dy]--;
            }
        }
    }
}
