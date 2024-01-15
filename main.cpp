#include <iostream>
#include <unistd.h>
#include <sys/inotify.h>

// close, read => unistd.h

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

int main() {
    int inotifyInstance;
    int watchDescriptor;
    char buffer[BUF_LEN];

    inotifyInstance = inotify_init();
    if (inotifyInstance < 0) {
        std::cerr << "inotify_init() failed" << std::endl;
        return 1;
    }

    watchDescriptor = inotify_add_watch(inotifyInstance, "/tmp", IN_CREATE | IN_DELETE | IN_MODIFY);
    if (watchDescriptor == -1) {
        std::cerr << "inotify_add_watch() failed" << std::endl;
        close(inotifyInstance);
        return 1;
    }

    while (true) {
        int length = read(inotifyInstance, buffer, BUF_LEN);
        if (length < 0) {
            std::cerr << "read() failed" << std::endl;
            break;
        }

        for (int i = 0; i < length;) {
            struct inotify_event *event = reinterpret_cast<struct inotify_event *>(&buffer[i]);
            if (event->len) {
                if (event->mask & IN_CREATE) {
                    std::cout << "file created: " << event->name << std::endl;
                } else if (event->mask & IN_DELETE) {
                    std::cout << "file deleted: " << event->name << std::endl;
                } else if (event->mask & IN_MODIFY) {
                    std::cout << "file modified: " << event->name << std::endl;
                }
                i += EVENT_SIZE + event->len;
            }
        }
    }

    inotify_rm_watch(inotifyInstance, watchDescriptor);
    close(inotifyInstance);

    return 0;
}
