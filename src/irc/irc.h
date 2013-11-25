/*
 * irc.h
 *
 *  Created on: 15 Jul 2011
 *      Author: tyler
 *
 *  Modified on: 24 Nov 2013
 *		Author: dmilith
 *
 */

#ifndef IRCBOT_H_
#define IRCBOT_H_


#include <QtCore>

#include "../globals/globals.h"
#include "../service_spawner/utils.h"
#include "../service_spawner/logger.h"

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>


#define MAXDATASIZE 512


class IrcNotification {

	public:
		IrcNotification();
		virtual ~IrcNotification();

		bool setup;
		void start();
		bool charSearch(QString toSearch, QString searchFor);

	private:
		QString port, message;
		int s; //the socket descriptor

		QString address = "irc.freenode.net";
		QString nick = "NICK git-bot\r\n"; // XXX: hardcoded
		QString usr = "USER git-bot gitbot botgit :Git Bot\r\n"; // XXX: hardcoded

		bool isConnected(QString buf);
		bool sendNotify(QString _msg);
		bool sendData(QString msg);
		void sendPong(QString buf);
		void msgHandle(QString buf);
		void receiveData(char buf[MAXDATASIZE]);
};


#endif /* IRCBOT_H_ */
