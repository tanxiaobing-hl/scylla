/*
 * Copyright (C) 2018 ScyllaDB
 *
 */

/*
 * This file is part of Scylla.
 *
 * Scylla is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scylla is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scylla.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "timestamp.hh"
#include "gc_clock.hh"
#include "sstables/m_format_read_helpers.hh"
#include "sstables/types.hh"
#include "mutation_partition.hh"

namespace sstables {

class liveness_info {
    const serialization_header& _header;
    api::timestamp_type _timestamp;
    gc_clock::duration _ttl;
    gc_clock::time_point _local_deletion_time;
    bool is_set() const {
        return _timestamp != api::missing_timestamp
               || _ttl != gc_clock::duration::zero()
               || _local_deletion_time != gc_clock::time_point::max();
    }
public:
    explicit liveness_info(const serialization_header& header)
        : _header(header)
    { }
    void reset() {
        _timestamp = api::missing_timestamp;
        _ttl = gc_clock::duration::zero();
        _local_deletion_time = gc_clock::time_point::max();
    }
    void set_timestamp(uint64_t delta) {
        _timestamp = parse_timestamp(_header, delta);
    }
    void set_ttl(uint32_t delta) {
        _ttl = parse_ttl(_header, delta);
    }
    void set_local_deletion_time(uint32_t delta) {
        _local_deletion_time = parse_expiry(_header, delta);
    }
    api::timestamp_type timestamp() const { return _timestamp; }
    gc_clock::duration ttl() const { return _ttl; }
    gc_clock::time_point local_deletion_time() const { return _local_deletion_time; }
    row_marker to_row_marker() const {
        if (!is_set()) {
            return row_marker();
        }
        return _ttl != gc_clock::duration::zero() || _local_deletion_time != gc_clock::time_point::max()
            ? row_marker(_timestamp, _ttl, _local_deletion_time)
            : row_marker(_timestamp);
    }
};

}
