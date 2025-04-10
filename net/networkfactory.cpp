#include "networkfactory.h"

NetworkHandler *NetworkFactory::createHandler(NetworkHandler::Mode mode, QObject *parent)
{
    qInfo() << "Network Factory handler creating... mode:"
            << NetworkHandler::mode2String(static_cast<int>(mode));
    switch (mode) {
    case NetworkHandler::Mode::TCP_CLIENT:
        return new TCPClientHandler(parent);
    default:
        return nullptr;
    }
}
