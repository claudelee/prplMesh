/* SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 * SPDX-FileCopyrightText: 2016-2020 the prplMesh contributors (see AUTHORS.md)
 *
 * This code is subject to the terms of the BSD+Patent license.
 * See LICENSE file for more details.
 */

#ifndef _BEEROCKS_SOCKET_EVENT_LOOP_H_
#define _BEEROCKS_SOCKET_EVENT_LOOP_H_

#include "beerocks_event_loop.h"
#include "network/socket.h"

#include <memory>
#include <unordered_map>

namespace beerocks {

/**
 * @brief ePoll based implementation of the EventLoop interface.
 * @see EventLoop
 * 
 * This class uses the Linux epoll APIs for monitoring the provided sockets for I/O operations.
 * Timeout operations are achieved by using the timerfd mechanism, which delivers timer 
 * expiration notifications via a file descriptor.
 */
class SocketEventLoop : public EventLoop<std::shared_ptr<Socket>, std::chrono::milliseconds> {
public:
    /**
     * @brief Class constructor.
     * 
     * Initializes an epoll file descriptor.
     * 
     * @param [in] timeout Sets the master timeout (in milliseconds) for the event loop.
     */
    explicit SocketEventLoop(TimeoutType timeout = TimeoutType::min());

    /**
     * @brief Class destructor.
     */
    virtual ~SocketEventLoop();

    /**
     * @see EventPoll::add_event
     */
    virtual bool add_event(SocketEventLoop::EventType socket,
                           SocketEventLoop::EventHandlers handlers,
                           TimeoutType timeout = TimeoutType::min()) override;

    /**
     * @see EventPoll::del_event
     */
    virtual bool del_event(SocketEventLoop::EventType socket) override;

    /**
     * @see EventPoll::run
     */
    virtual int run() override;

    /**
     * @see EventPoll::die
     */
    virtual void die() override;

protected:
    /**
     * @brief Main event loop method.
     * 
     * Executes the epoll_wait() function and processes ocurred events.
     * 
     * @return -1 on critical errors
     * @return  0 on timeout without any socket events
     * @return >0 number of socket events processed during the call to this method.
     */
    virtual int event_loop();

private:
    /**
     * epoll file descriptor.
     */
    int m_epoll_fd = -1;

    /**
     * Event loop master timeout (used for the epoll_wait function).
     */
    TimeoutType m_timeout = TimeoutType::min();

    /**
     * Flag to mark whether the event loop is running.
     */
    bool m_running = false;

    /**
     * @brief Data structure representing a socket added to the poll.
     * This structure groups all the information required for processing socket events.
     */
    struct EventData {
        /**
         * Socket event handler functions structure.
         */
        SocketEventLoop::EventHandlers handlers;

        /**
         * Shared pointer to the socket object.
         */
        SocketEventLoop::EventType socket = nullptr;

        /**
         * timer file descriptor.
         */
        int timerfd = -1;

        /**
         * Socket timeout value in milliseconds.
         */
        TimeoutType timeout_value = TimeoutType::min();
    };

    /**
     * Map file descriptors to EventData structure instances.
     */
    std::unordered_map<int, std::shared_ptr<EventData>> m_fd_to_event_data;
};

} // namespace beerocks

#endif // _BEEROCKS_SOCKET_EVENT_LOOP_H_
