#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>
#include <vector>

class Epoll{
public:
    Epoll();
    ~Epoll();

    bool add_fd(int fd, uint32_t events);
    bool mod_fd(int fd, uint32_t events);
    bool del_fd(int fd);    

    int wait(struct epoll_event* events, int max_events, int timeout = -1);

    bool is_valid(){return is_valid_;}
    int get_fd() const { return epoll_fd_; }
private:
    int epoll_fd_;
    bool is_valid_;

};


#endif