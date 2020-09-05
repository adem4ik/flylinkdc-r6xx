/*

Copyright (c) 2014, 2016-2020, Arvid Norberg
Copyright (c) 2016, Alden Torres
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef SESSION_VIEW_HPP_
#define SESSION_VIEW_HPP_

#include <cstdint>
#include <vector>

#include "libtorrent/session_stats.hpp"
#include "libtorrent/span.hpp"
#include "libtorrent/time.hpp"

struct session_view
{
	session_view();

	void set_pos(int pos);
	void set_width(int width);

	int pos() const;

	int height() const;

	void render();

	void update_counters(lt::span<std::int64_t const> stats_counters, lt::clock_type::time_point t);

private:

	int m_position = 0;
	int m_width = 80;

	// there are two sets of counters. the current one and the last one. This
	// is used to calculate rates
	std::vector<std::int64_t> m_cnt[2];

	std::int64_t value(int idx) const;
	std::int64_t prev_value(int idx) const;

	// the timestamps of the counters in m_cnt[0] and m_cnt[1]
	// respectively.
	lt::clock_type::time_point m_timestamp[2];

	int const m_queued_bytes_idx = lt::find_metric_idx("disk.queued_write_bytes");
	int const m_wasted_bytes_idx = lt::find_metric_idx("net.recv_redundant_bytes");
	int const m_failed_bytes_idx = lt::find_metric_idx("net.recv_failed_bytes");
	int const m_num_peers_idx = lt::find_metric_idx("peer.num_peers_connected");
	int const m_recv_idx = lt::find_metric_idx("net.recv_bytes");
	int const m_sent_idx = lt::find_metric_idx("net.sent_bytes");
	int const m_unchoked_idx = lt::find_metric_idx("peer.num_peers_up_unchoked");
	int const m_unchoke_slots_idx = lt::find_metric_idx("ses.num_unchoke_slots");
	int const m_limiter_up_queue_idx = lt::find_metric_idx("net.limiter_up_queue");
	int const m_limiter_down_queue_idx = lt::find_metric_idx("net.limiter_down_queue");
	int const m_queued_writes_idx = lt::find_metric_idx("disk.num_write_jobs");
	int const m_queued_reads_idx = lt::find_metric_idx("disk.num_read_jobs");

	int const m_num_blocks_read_idx = lt::find_metric_idx("disk.num_blocks_read");
	int const m_blocks_in_use_idx = lt::find_metric_idx("disk.disk_blocks_in_use");
	int const m_blocks_written_idx = lt::find_metric_idx("disk.num_blocks_written");
	int const m_write_ops_idx = lt::find_metric_idx("disk.num_write_ops");

	int const m_utp_idle = lt::find_metric_idx("utp.num_utp_idle");
	int const m_utp_syn_sent = lt::find_metric_idx("utp.num_utp_syn_sent");
	int const m_utp_connected = lt::find_metric_idx("utp.num_utp_connected");
	int const m_utp_fin_sent = lt::find_metric_idx("utp.num_utp_fin_sent");
	int const m_utp_close_wait = lt::find_metric_idx("utp.num_utp_close_wait");

	int const m_queued_tracker_announces = lt::find_metric_idx("tracker.num_queued_tracker_announces");
};

#endif

