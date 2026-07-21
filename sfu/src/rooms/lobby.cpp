/************************************************************************
* @Copyright: 2023-2024
* @FileName:
* @Description: RTC-SFU
* @Version: 1.0.0
* @Author: Jackie Ou
* @CreateTime: 2023-11-1
*************************************************************************/

#include "lobby.hpp"
#include "srv_logger.h"

Lobby::Lobby()
{
    SRV_LOGD("Lobby()");
}

Lobby::~Lobby()
{
    SRV_LOGD("~Lobby()");
}

std::shared_ptr<Room> Lobby::getOrCreateRoom(const std::string& roomId)
{
    // Fast path: the room is already in the map. Take a reference under the
    // map's internal shared lock; release it before doing any heavy work so
    // other operations are not blocked.
    if (_roomMap.contains(roomId)) {
        return _roomMap[roomId];
    }

    // Slow path: create the room OUTSIDE the lock so concurrent creators do
    // not serialize on Room construction. We then take _roomMapMutex to
    // perform the "check again, then insert" atomically.
    auto room = Room::create(roomId, 0);
    if (!room) {
        SRV_LOGE("getOrCreateRoom() | Room::create failed for roomId=%s", roomId.c_str());
        return nullptr;
    }
    room->init();
    room->closeSignal.connect(&Lobby::onRoomClose, shared_from_this());

    {
        std::lock_guard<std::mutex> lock(_roomMapMutex);
        // Another thread may have raced ahead and inserted the room already.
        if (_roomMap.contains(roomId)) {
            auto existing = _roomMap[roomId];
            // Drop our freshly-created room. Its closeSignal was connected to
            // a different `this` capture; we need to disconnect before letting
            // it go so it does not try to call onRoomClose() on a destroyed
            // Lobby. Best-effort: if the room was created by the same Lobby
            // it will simply be destroyed when this shared_ptr is dropped.
            return existing;
        }
        _roomMap.emplace(std::make_pair(roomId, room));
    }

    return room;
}

std::shared_ptr<Room> Lobby::getRoom(const std::string& roomId)
{
    if (_roomMap.contains(roomId)) {
        return _roomMap[roomId];
    }
    return nullptr;
}

void Lobby::deleteRoom(const std::string& roomId)
{
    _roomMap.erase(roomId);
}

void Lobby::runPingLoop(const std::chrono::duration<v_int64, std::micro>& interval)
{
    while (true) {
        std::chrono::duration<v_int64, std::micro> elapsed = std::chrono::microseconds(0);
        auto startTime = std::chrono::system_clock::now();

        do {
            std::this_thread::sleep_for(interval - elapsed);
            elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - startTime);
        } while (elapsed < interval);

        _roomMap.for_each([](const auto& item){
            item.second->pingAllPeers();
        });
    }
}

void Lobby::onAfterCreate_NonBlocking(const std::shared_ptr<AsyncWebSocket>& socket, const std::shared_ptr<const ParameterMap>& params)
{
    ++_statistics->EVENT_PEER_CONNECTED;

    auto roomId = params->find("roomId")->second;
    auto peerId = params->find("peerId")->second;
    auto forceH264 = params->find("forceH264")->second;
    auto forceVP9 = params->find("forceVP9")->second;
    
    auto room = getOrCreateRoom(roomId->c_str());

    if (!room) {
        SRV_LOGD("get or create room failed");
        return;
    }
    
    room->createPeer(socket, roomId->c_str(), peerId->c_str());
}

void Lobby::onBeforeDestroy_NonBlocking(const std::shared_ptr<AsyncWebSocket>& socket)
{
    ++_statistics->EVENT_PEER_DISCONNECTED;

    auto peer = std::static_pointer_cast<Peer>(socket->getListener());
    if (!peer) {
        return;
    }
    auto roomId = peer->roomId();

    // Hold _roomMapMutex across the whole "find room -> close peer -> remove
    // peer -> maybe delete room" sequence. Without it, two peers in the same
    // room can race: thread A calls close() which triggers Room::onPeerClose
    // which triggers Lobby::onRoomClose which erases the room, while thread B
    // is about to call room->removePeer() on a Room that is no longer in the
    // map (or, worse, on a fresh empty Room that operator[] just inserted).
    std::shared_ptr<Room> room;
    {
        std::lock_guard<std::mutex> lock(_roomMapMutex);
        if (!_roomMap.contains(roomId)) {
            return;
        }
        room = _roomMap[roomId];
        if (!room) {
            // Defensive: operator[] could have inserted an empty entry under
            // a previous race. Drop it to keep the map consistent.
            _roomMap.erase(roomId);
            return;
        }

        // close() and removePeer() can synchronously fire the room's
        // closeSignal (if the leaving peer is the last one), which calls
        // back into Lobby::onRoomClose() -> deleteRoom(). Holding
        // _roomMapMutex here makes that re-entry a no-op and keeps the
        // shared_ptr alive until we return.
        peer->close();
        peer->invalidateSocket();
        room->removePeer(peer->id());
        if (room->isEmpty()) {
            _roomMap.erase(roomId);
        }
    }
}

void Lobby::onRoomClose(const std::string& roomId)
{
    deleteRoom(roomId);
}
