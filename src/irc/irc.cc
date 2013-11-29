/*
 * irc.cc
 *
 *  Created on: 15 Jul 2011
 *      Author: tyler
 *
 *  Modified on: 24 Nov 2013
 *		Author: dmilith
 *
 */

#include "irc.h"


IrcNotification::IrcNotification() {
}


IrcNotification::~IrcNotification() {
	close(s);
}


void IrcNotification::start() {
	struct addrinfo hints, *servinfo;

	//Setup run with no errors
	setup = true;
	port = "6667";

	//Ensure that servinfo is clear
	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_INET; // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

	//Setup the structs if error print why
	int res;
	if ((res = getaddrinfo(address.toUtf8(), port.toUtf8(), &hints, &servinfo)) != 0) {
		setup = false;
		fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(res));
	}


	//setup the socket
	if ((s = socket(servinfo->ai_family,servinfo->ai_socktype,servinfo->ai_protocol)) == -1) {
		perror("client: socket");
	}

	//Connect
	if (connect(s,servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
		close (s);
		perror("Client Connect");
	}

	//We dont need this anymore
	freeaddrinfo(servinfo);

	//Recv some data
	int count = 0;
	char buf[MAXDATASIZE];

	while (true) {
		count++;
		logWarn() << "C:" << count;
		switch (count) {
			case 3: {
				//after 3 recives send data to server (as per IRC protacol)
				sendData(nick);
				sendData(usr);
				sendData("PRIVMSG NickServ :IDENTIFY git-bot git-bot-666\n");
				// sendData("JOIN #verknowsys\n");
			} break;

			case 4: {
				QString notificationsDir = QString(getenv("HOME")) + SOFTWARE_DATA_DIR + NOTIFICATIONS_DATA_DIR + "/";
				if (getuid() == 0) {
			        notificationsDir = QString(SYSTEM_USERS_DIR) + SOFTWARE_DATA_DIR + NOTIFICATIONS_DATA_DIR + "/";
			    }
			    logInfo() << "Looking for ERROR notifications inside" << notificationsDir;

			    QStringList notifications = QDir(notificationsDir).entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time);
			    logInfo() << "Current list of notifications:" << notifications;
			    ::reverse(notifications.begin(), notifications.end());
			    Q_FOREACH(QString notify, notifications) {
			    	if (notify.endsWith(".error")) {
			    		QString contents = readFileContents(notificationsDir + notify).trimmed();
			    		if (not contents.startsWith("-")) {
					    	sendNotify("ERROR: " + contents);
					    	writeToFile(notificationsDir + notify, "-" + contents);
			    		}
			    	}
			    }
				sendData("QUIT [Bye]\n");

			} break;

		}

		receiveData(buf);
	}
}


void IrcNotification::receiveData(char buf[MAXDATASIZE]) {
	int numbytes;

	// Recv & print Data
	numbytes = recv(s, buf, MAXDATASIZE - 1, 0);
	buf[numbytes] = '\0';
	logInfo() << QString(buf);

	if (charSearch(QString(buf), "PING"))
		sendPong(QString(buf));

	if (numbytes == 0) {
		logInfo() << "----------------------CONNECTION CLOSED---------------------------";
		exit(0);
	}
}


bool IrcNotification::charSearch(QString toSearch, QString searchFor) {
	int len = toSearch.length();
	int forLen = searchFor.length(); // The length of the searchfor field

	//Search through each char in toSearch
	for (int i = 0; i < len;i++) {
		//If the active char is equil to the first search item then search toSearch
		if (searchFor[0] == toSearch[i]) {
			bool found = true;
			//search the char array for search field
			for (int x = 1; x < forLen; x++) {
				if (toSearch[i+x]!=searchFor[x]) {
					found = false;
				}
			}
			//if found return true;
			if (found == true)
				return true;
		}
	}
	return false;
}


bool IrcNotification::isConnected(QString buf) { //returns true if "/MOTD" is found in the input strin
	//If we find /MOTD then its ok join a channel
	if (charSearch(buf, "/MOTD") == true)
		return true;
	else
		return false;
}


bool IrcNotification::sendNotify(QString _msg) {
	return sendData("NOTICE #verknowsys :@" + QHostInfo::localHostName() + ": " + _msg + "\n");
}


bool IrcNotification::sendData(QString _msg) { //Send some data
	QString msg = "\n" + _msg;
	//Send some data
	int len = msg.length();
	int bytes_sent = send(s, msg.toUtf8().constData(), len, 0);

	if (bytes_sent == 0)
		return false;
	else
		return true;
}


void IrcNotification::sendPong(QString buf) {
	//Get the reply address
	//loop through bug and find the location of PING
	//Search through each char in toSearch

	QString toSearch = "PING ";

	for (int i = 0; i < buf.length(); i++) {
			//If the active char is equil to the first search item then search toSearch
			if (buf[i] == toSearch[0]) {
				bool found = true;
				//search the char array for search field
				for (int x = 1; x < 4; x++) {
					if (buf[i+x]!=toSearch[x]) {
						found = false;
					}
				}

				//if found return true;
				if (found == true) {
					int count = 0;
					//Count the chars
					for (int x = (i + toSearch.length()); x < buf.length(); x++) {
						count++;
					}

					//Create the new char array
					QString returnHost;
					returnHost[0]='P';
					returnHost[1]='O';
					returnHost[2]='N';
					returnHost[3]='G';
					returnHost[4]=' ';

					count = 0;
					//set the hostname data
					for (int x = (i + toSearch.length()); x < buf.length(); x++) {
						returnHost[count+5]=buf[x];
						count++;
					}

					//send the pong
					if (sendData(returnHost)) {
						logDebug() << "Ping Pong";
					}
					return;
				}
			}
		}
}


void IrcNotification::msgHandle(QString buf) {
	/*
	 * TODO: add you code to respod to commands here
	 * the example below replys to the command hi scooby
	 */
	if (charSearch(buf, "help")) {
		sendData("PRIVMSG #verknowsys :no help yet\r\n");
	}
}

