#ifndef NETWORKFACTORY_H
#define NETWORKFACTORY_H

#include "networkhandler.h"

class NetworkFactory
{
public:
    static NetworkHandler *createHandler(NetworkHandler::Mode mode, QObject *parent = nullptr);
};

#endif // NETWORKFACTORY_H
