#include <iostream>
#include <string.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <arpa/inet.h>


using namespace std;

void sys_err(const char *str){
    perror(str);
    exit(1);
}

void read_cb(bufferevent *bev,void *arg){
    char buf[1024] = {0};
    size_t read_len = bufferevent_read(bev,buf,sizeof(buf));
    buf[read_len] = 0;
    cout<< "recv data: "<<buf<<endl;
    cout<< "--------------------------"<<endl;
    bufferevent_write(bev,buf,read_len);
}

void write_cb(bufferevent *bev,void *arg){
    cout<< "write_cb"<<endl;
}

void event_cb(bufferevent *bev,short events,void *arg){
    if(events & BEV_EVENT_EOF){
        cout<< "connection closed"<<endl;
    }else if(events & BEV_EVENT_ERROR){
        cout<< "some other error"<<endl;
    }
    bufferevent_free(bev);
}


void accept_cb(struct evconnlistener * listener, evutil_socket_t fd, 
    struct sockaddr * addr, int socklen, void *arg){
    event_base *base = (event_base *)arg;

    bufferevent *bev = bufferevent_socket_new(base,fd,BEV_OPT_CLOSE_ON_FREE);
    if(!bev){
        sys_err("bufferevent_socket_new");
    }

    bufferevent_setcb(bev, read_cb, write_cb, event_cb, nullptr);
    bufferevent_enable(bev,EV_READ|EV_WRITE);

    cout<< "new connection"<<endl;
    cout<< "ip: "<<inet_ntoa(((sockaddr_in *)addr)->sin_addr)<<" "<<"port: "
        <<ntohs(((sockaddr_in *)addr)->sin_port)<<endl;
    cout<< "--------------------------"<<endl;
}


int main(){
    auto base = event_base_new();
    if(!base){
        sys_err("event_base_new");
    }

    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(8080);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    auto listener = evconnlistener_new_bind(base,accept_cb,base,
        LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE,-1,(struct sockaddr *)&sin,sizeof(sin));
    if(!listener){
        sys_err("evconnlistener_new_bind");
    }

    cout<< "server start"<<endl;
    event_base_dispatch(base);
    evconnlistener_free(listener);
    event_base_free(base);
    return 0;
}