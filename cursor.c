#include <X11/Xlib.h>
#include <stdbool.h> // 用于使用bool类型
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // 用于usleep函数

typedef struct Pos {
  int x, y, dx, dy;
  struct Pos *next;
} Pos;

#define MAX_POS 24
#define MAX_DELTA 400

static Pos Poss[MAX_POS];

bool isJiggled() {
  int changeNumX = 0;
  int max_x = Poss[0].x, min_x = Poss[0].x, max_y = Poss[0].y,
      min_y = Poss[0].y;

  for (int i = 0; i < MAX_POS; i++) {
    if ((Poss[i].dx > 0 && Poss[i].next->dx < 0) ||
        (Poss[i].dx < 0 && Poss[i].next->dx > 0)) {
      changeNumX++;
    }

    if (Poss[i].x > max_x)
      max_x = Poss[i].x;
    if (Poss[i].x < min_x)
      min_x = Poss[i].x;
    if (Poss[i].y > max_y)
      max_y = Poss[i].y;
    if (Poss[i].y < min_y)
      min_y = Poss[i].y;
  }

  return changeNumX > 2 && (max_x - min_x < MAX_DELTA) &&
         (max_y - min_y < MAX_DELTA / 2);
}

int main() {
  Display *disp;
  Window root_window;
  int screen;
  Pos *index = &Poss[0];
  Pos *prev = &Poss[MAX_POS - 1];

  // 创建10个Pos结构体，并头尾相连
  for (int i = 0; i < MAX_POS; i++) {
    Poss[i].next = &Poss[(i + 1) % MAX_POS];
    Poss[i].x = 0;
    Poss[i].y = 0;
    Poss[i].dx = 0;
    Poss[i].dy = 0;
  }

  // 打开与X服务器的连接
  disp = XOpenDisplay(NULL);
  if (disp == NULL) {
    fprintf(stderr, "无法打开X显示\n");
    return 1;
  }

  // 获取屏幕信息
  screen = DefaultScreen(disp);
  root_window = RootWindow(disp, screen);

  Window child_window;
  int win_x, win_y;
  unsigned int mask;

  while (1) {
    // 获取光标位置
    bool result = XQueryPointer(disp, root_window, &child_window, &child_window,
                                &index->x, &index->y, &win_x, &win_y, &mask);

    if (result) {
      // 计算位置变化
      index->dx = index->x - prev->x;
      index->dy = index->y - prev->y;

      // 检测鼠标晃动
      if (isJiggled()) {
        system("cursor 140");
        usleep(30000);
      } else {
        system("cursor 48");
      }

      // 更新前一个位置
      prev = index;
    }

    // 更新当前索引
    index = index->next;

    // 等待100毫秒
    usleep(10000);
  }

  // 关闭与X服务器的连接
  XCloseDisplay(disp);

  return 0;
}
