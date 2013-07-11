/**
 *  @author dmilith, teamon
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "panel.h"

template<class T>
void ScrollList<T>::display(){
    int s = this->items->size();
    if(current >= s) current = s - 1; // handle item removal

    displayHeader();

    int n = min(maxRows, s);
    int i=0;
    for(i=0; i<n; i++){
        int k = first + i;
        displayItem(this->items->at(k), i, k, current == k);
    }
    for(; i < maxRows; i++){
        displayEmptyItem(i);
    }

    displayFooter();
}

template<class T>
void ScrollList<T>::setItems(QList<T> * items){
    this->items = items;
}

template<class T>
void ScrollList<T>::reset(int maxRows){
    this->current = 0;
    this->first = 0;
    this->maxRows = maxRows;
}

template<class T>
void ScrollList<T>::key(int ch){
    switch(ch){
        case KEY_UP:
            if(current > 0) current--;
            if(current < first) first = current;
            break;

        case KEY_DOWN:
            int s = this->items->size();
            if(current < s - 1) current++;
            else current = s - 1;

            if(current >= first + maxRows) first++;
            break;
    }
}

template<class T>
T ScrollList<T>::currentItem(){
    if(items->empty()) {
        return NULL;
    } else {
        if(current < 0) current = 0; // Fix for case when ading first service
        if(current >= items->size()) current = items->size() - 1;
        return items->at(current);
    }
}


AvailableServicesList::AvailableServicesList(QList<QString> * items, int maxRows, WINDOW * win, QString aHeader): ScrollList(items, maxRows, win){
    this->all = QList<QString>(*items);
    this->header = aHeader;
    setName("");
}

void AvailableServicesList::displayHeader(){
    wattron(win, COLOR_PAIR(6));
    box(win, '|', '-');
    wattroff(win, COLOR_PAIR(6));

    wattron(win, COLOR_PAIR(2));
    mvwprintw(win, 1, 2, header.toUtf8());
    wattron(win, COLOR_PAIR(2));
}

void AvailableServicesList::displayFooter(){
    mvwprintw(win, 1, 18, "%-25s", name.toUtf8().data()); // prompt
    wmove(win, 1, 18 + name.length());
}

void AvailableServicesList::setName(QString name){
    this->name = name;
    this->current = 0;
    this->first = 0;

    items->clear();
    Q_FOREACH (QString s, all) {
            if(s.toLower().contains(name.toLower())){
                    *items << s;
            }
    }
}

void AvailableServicesList::displayItem(QString item, int i, int num, bool current){
    if(current){
        wattron(win, COLOR_PAIR(4));
    } else {
        wattroff(win, COLOR_PAIR(4));
    }

    mvwprintw(win, i+2, 2, "%2d", num+1);
    mvwprintw(win, i+2, 4, " %-40s", item.toUtf8().data());
    wattroff(win, COLOR_PAIR(4));
}

void AvailableServicesList::displayEmptyItem(int i){
    mvwprintw(win, i+2, 2, "%42s", "");
}


void ServicesList::displayHeader(){
    mvwprintw(win, 3, 0, " # Name                        PID Address               Status        Flags   Autostart");
}

void ServicesList::displayItem(PanelService * service, int i, int num, bool current){
    QString status;
    int color;

    switch(service->status){
        case SERVICE_STATUS_INSTALLING:
            status = "Installing...";
            color = C_STATUS_INSTALLING;
            break;
        case SERVICE_STATUS_VALIDATING:
            status = "Validating...";
            color = C_STATUS_VALIDATING;
            break;
        case SERVICE_STATUS_CONFIGURING:
            status = "Configuring...";
            color = C_STATUS_CONFIGURING;
            break;
        case SERVICE_STATUS_WORKING:
            status = "Working...";
            color = C_STATUS_WORKING;
            break;
        case SERVICE_STATUS_RUNNING:
            status = "Running";
            color = C_STATUS_RUNNING;
            break;
        case SERVICE_STATUS_ERRORS:
            status = "Errors";
            color = C_STATUS_ERRORS;
            break;
        case SERVICE_STATUS_STOPPED:
            status = "Stopped";
            color = C_STATUS_STOPPED;
            break;
    }

    if(current) color = C_CURRENT;

    wattron(win, color);

    int x = 0, y = i + 4;

    /* # */
    mvwprintw(win, y, x, "%2d", num+1);
    x += 2;

    /* name */
    mvwprintw(win, y, x, " %-26s", service->name.toUtf8().data());
    x += 27;

    /* pid */
    mvwprintw(win, y, x, "%5s", service->pid.toUtf8().data());
    x += 5;

    /* domain:port */
    mvwprintw(win, y, x, " %-21s", (service->domain + ":" + service->port).toUtf8().data());
    x += 22;

    /* flags & status */
    mvwprintw(win, y, x, " %-14s", status.toUtf8().data());
    x += 14;
    mvwprintw(win, y, x, " %s", service->flags);
    x += 6;

    if(service->autostart)  mvwprintw(win, y, x, "   YES       ");
    else    mvwprintw(win, y, x, "             ");

    wattroff(win, color);
}

void ServicesList::displayEmptyItem(int i){
    mvwprintw(win, i+4, 0, "%100s", "");
}

void ServicesList::setCurrent(QString selected){
    for(int i=0; i<items->size(); i++){
        if(items->at(i)->name == selected){
            this->current = i;
            break;
        }
    }
}


template class ScrollList<QString>;
template class ScrollList<PanelService *>;
