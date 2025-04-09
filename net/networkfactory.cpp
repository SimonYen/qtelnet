#include "networkfactory.h"

NetworkHandler *NetworkFactory::createHandler(NetworkHandler::Mode mode, QObject *parent)
{
    switch (mode) {
    case NetworkHandler::Mode::TCP_CLIENT:
        return new TCPClientHandler(parent);
    default:
        return nullptr;
    }
}
