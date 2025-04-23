/*
 * libevdev testing -- creates a virtual mouse and moves it.
 * Pulled from https://kernel.org/doc/html/v4.12/input/uinput.html#mouse-movements
 *
 * Must be run with `sudo` (udev rules will be added later).
 * Must be compiled with `gcc ./libevdev.c $(pkg-config --libs libevdev)` on NixOS; for some reason, gcc isn't finding it.
 * TODO:
 * - Look into libevdev_uinput.h as an alternative to uinput.h -- Are there convenience functions/wrappers that I can take advantage of?
*/
#include <errno.h>
#include <fcntl.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  struct libevdev *dev = NULL;
  int fd;
  int rc = 1;

  fd = open("/dev/input/by-path/platform-i8042-serio-1-event-mouse", O_RDONLY | O_NONBLOCK);
  rc = libevdev_new_from_fd(fd, &dev);
  if (rc < 0) {
    fprintf(stderr, "Failed to init libevdev (%s)\n", strerror(-rc));
    exit(1);
  }
  printf("Input device name: \"%s\"\n", libevdev_get_name(dev));
  printf("Input device ID: bus %#x vendor %#x product %#x\n",
         libevdev_get_id_bustype(dev), libevdev_get_id_vendor(dev),
         libevdev_get_id_product(dev));
  if (!libevdev_has_event_type(dev, EV_REL) ||
      !libevdev_has_event_code(dev, EV_KEY, BTN_LEFT)) {
    printf("This device does not look like a mouse\n");
    exit(1);
  }

  do {
    struct input_event ev;
    rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
    if (rc == 0)
      printf("Event: %s %s %d\n", libevdev_event_type_get_name(ev.type),
             libevdev_event_code_get_name(ev.type, ev.code), ev.value);
  } while (rc == 1 || rc == 0 || rc == -EAGAIN);

  return EXIT_SUCCESS;
}
