#include <dirent.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <unistd.h>

#include "config.h"

volatile bool thread_stop = false;

bool is_suitable(int fd);
void *autoclick(void *arg);

int main() {
  DIR *input_dir = opendir("/dev/input");
  if (input_dir == NULL) {
    fprintf(stderr, "cannot open /dev/input\n");
    exit(EXIT_FAILURE);
  }

  struct pollfd *fds = malloc(sizeof(struct pollfd) * 128);
  struct dirent *pDirent;
  int fds_c = 0;
  while ((pDirent = readdir(input_dir)) != NULL) {
    if (pDirent->d_type == 2 /* DT_CHR */) {
      char file_path[128] = "/dev/input/";
      strcat(file_path, pDirent->d_name);
      int fd = open(file_path, O_RDONLY | O_NONBLOCK);
      if (is_suitable(fd)) {
        printf("found event with support for hotkey (%d): %s\n", HOTKEY, file_path);
        fds[fds_c].fd = fd;
        fds[fds_c].events = POLLIN;
        fds_c++;
      } else {
        close(fd);
      }
    }
  }
  closedir(input_dir);

  fds = realloc(fds, fds_c * sizeof(fds[0]));

  int uinput = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

  ioctl(uinput, UI_SET_EVBIT, EV_KEY);
  ioctl(uinput, UI_SET_KEYBIT, BTN_LEFT);

  struct uinput_setup usetup = {0};
  usetup.id.bustype = BUS_USB;
  usetup.id.vendor = 0x1234;
  usetup.id.product = 0x5678;
  strcpy(usetup.name, "Autoclicker");

  ioctl(uinput, UI_DEV_SETUP, &usetup);
  ioctl(uinput, UI_DEV_CREATE);

  bool toggle = false;

  pthread_t thread;

  printf("ready\n");
  struct input_event ie = {0};
  for (;;) {
    int ret = poll(fds, fds_c, -1);
    if (ret > 0) {
      for (int i = 0; i < fds_c; i++) {
        if (fds[i].revents & POLLIN) {
          read(fds[i].fd, &ie, sizeof(ie));
          if (ie.code == HOTKEY && ie.type == 1 && ie.value == 0) {
            if (toggle) {
              thread_stop = true;
              pthread_join(thread, NULL);
            } else {
              pthread_create(&thread, NULL, autoclick, &uinput);
            }
            toggle = !toggle;
            printf("%s\n", toggle ? "Enabled" : "Disabled");
          }
        }
      }
    }
  }

  ioctl(uinput, UI_DEV_DESTROY);
  close(uinput);

  return 0;
}

bool is_suitable(int fd) {
  size_t nchar = KEY_MAX / 8 + 1;
  unsigned char bits[nchar];
  ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(bits)), &bits);
  return bits[HOTKEY / 8] & (1 << (HOTKEY % 8));
}

const struct input_event btn_down = {
    .type = EV_KEY,
    .code = BTN_LEFT,
    .value = 1,
};
const struct input_event btn_up = {
    .type = EV_KEY,
    .code = BTN_LEFT,
    .value = 0,
};
const struct input_event report = {
    .type = EV_SYN,
    .code = SYN_REPORT,
    .value = 0,
};

void *autoclick(void *arg) {
  const int uinput = *(int *)arg;

  for (;;) {
    const struct timespec time = {.tv_sec = 0, .tv_nsec = DELAY_MS * 1000000};
    if (thread_stop) {
      thread_stop = false;
      pthread_exit(0);
    }
    write(uinput, &btn_down, sizeof(btn_down));
    write(uinput, &report, sizeof(report));
    sleep(0);
    write(uinput, &btn_up, sizeof(btn_up));
    write(uinput, &report, sizeof(report));
    nanosleep(&time, NULL);
  }
}
