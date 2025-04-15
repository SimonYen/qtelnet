#include "networkfactory.h"
#include "tcpclienthandler.h"
#include "tcpserverhandler.h"

NetworkHandler *NetworkFactory::createHandler(NetworkHandler::Mode mode, QObject *parent)
{
    qInfo() << "Network Factory handler creating... mode:"
            << NetworkHandler::mode2String(static_cast<int>(mode));
    switch (mode) {
    case NetworkHandler::Mode::TCP_CLIENT:
        return new TCPClientHandler(parent);
    case NetworkHandler::Mode::TCP_SERVER:
        return new TCPServerHandler(parent);
    default:
        return nullptr;
    }
}
