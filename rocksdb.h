#line 1 "/home/evan/source/rocksdb/include/rocksdb/table_properties.h"
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include <stdint.h>
#include <string>
#include <map>
#line 1 "/home/evan/source/rocksdb/include/rocksdb/status.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// A Status encapsulates the result of an operation.  It may indicate success,
// or it may indicate an error with an associated error message.
//
// Multiple threads can invoke const methods on a Status without
// external synchronization, but if any of the threads may call a
// non-const method, all threads accessing the same Status must use
// external synchronization.

#ifndef STORAGE_ROCKSDB_INCLUDE_STATUS_H_
#define STORAGE_ROCKSDB_INCLUDE_STATUS_H_

#include <string>
#line 1 "/home/evan/source/rocksdb/include/rocksdb/slice.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// Slice is a simple structure containing a pointer into some external
// storage and a size.  The user of a Slice must ensure that the slice
// is not used after the corresponding external storage has been
// deallocated.
//
// Multiple threads can invoke const methods on a Slice without
// external synchronization, but if any of the threads may call a
// non-const method, all threads accessing the same Slice must use
// external synchronization.

#ifndef STORAGE_ROCKSDB_INCLUDE_SLICE_H_
#define STORAGE_ROCKSDB_INCLUDE_SLICE_H_

#include <assert.h>
#include <cstdio>
#include <stddef.h>
#include <string.h>
#include <string>

namespace rocksdb {

class Slice {
 public:
  // Create an empty slice.
  Slice() : data_(""), size_(0) { }

  // Create a slice that refers to d[0,n-1].
  Slice(const char* d, size_t n) : data_(d), size_(n) { }

  // Create a slice that refers to the contents of "s"
  /* implicit */
  Slice(const std::string& s) : data_(s.data()), size_(s.size()) { }

  // Create a slice that refers to s[0,strlen(s)-1]
  /* implicit */
  Slice(const char* s) : data_(s), size_(strlen(s)) { }

  // Create a single slice from SliceParts using buf as storage.
  // buf must exist as long as the returned Slice exists.
  Slice(const struct SliceParts& parts, std::string* buf);

  // Return a pointer to the beginning of the referenced data
  const char* data() const { return data_; }

  // Return the length (in bytes) of the referenced data
  size_t size() const { return size_; }

  // Return true iff the length of the referenced data is zero
  bool empty() const { return size_ == 0; }

  // Return the ith byte in the referenced data.
  // REQUIRES: n < size()
  char operator[](size_t n) const {
    assert(n < size());
    return data_[n];
  }

  // Change this slice to refer to an empty array
  void clear() { data_ = ""; size_ = 0; }

  // Drop the first "n" bytes from this slice.
  void remove_prefix(size_t n) {
    assert(n <= size());
    data_ += n;
    size_ -= n;
  }

  // Return a string that contains the copy of the referenced data.
  std::string ToString(bool hex = false) const;

  // Three-way comparison.  Returns value:
  //   <  0 iff "*this" <  "b",
  //   == 0 iff "*this" == "b",
  //   >  0 iff "*this" >  "b"
  int compare(const Slice& b) const;

  // Return true iff "x" is a prefix of "*this"
  bool starts_with(const Slice& x) const {
    return ((size_ >= x.size_) &&
            (memcmp(data_, x.data_, x.size_) == 0));
  }

  // Compare two slices and returns the first byte where they differ
  size_t difference_offset(const Slice& b) const;

 // private: make these public for rocksdbjni access
  const char* data_;
  size_t size_;

  // Intentionally copyable
};

// A set of Slices that are virtually concatenated together.  'parts' points
// to an array of Slices.  The number of elements in the array is 'num_parts'.
struct SliceParts {
  SliceParts(const Slice* _parts, int _num_parts) :
      parts(_parts), num_parts(_num_parts) { }
  SliceParts() : parts(nullptr), num_parts(0) {}

  const Slice* parts;
  int num_parts;
};

inline bool operator==(const Slice& x, const Slice& y) {
  return ((x.size() == y.size()) &&
          (memcmp(x.data(), y.data(), x.size()) == 0));
}

inline bool operator!=(const Slice& x, const Slice& y) {
  return !(x == y);
}

inline int Slice::compare(const Slice& b) const {
  const size_t min_len = (size_ < b.size_) ? size_ : b.size_;
  int r = memcmp(data_, b.data_, min_len);
  if (r == 0) {
    if (size_ < b.size_) r = -1;
    else if (size_ > b.size_) r = +1;
  }
  return r;
}

inline size_t Slice::difference_offset(const Slice& b) const {
  size_t off = 0;
  const size_t len = (size_ < b.size_) ? size_ : b.size_;
  for (; off < len; off++) {
    if (data_[off] != b.data_[off]) break;
  }
  return off;
}

}  // namespace rocksdb

#endif  // STORAGE_ROCKSDB_INCLUDE_SLICE_H_
#line 21 "/home/evan/source/rocksdb/include/rocksdb/status.h"

namespace rocksdb {

class Status {
 public:
  // Create a success status.
  Status() : code_(kOk), subcode_(kNone), state_(nullptr) {}
  ~Status() { delete[] state_; }

  // Copy the specified status.
  Status(const Status& s);
  void operator=(const Status& s);
  bool operator==(const Status& rhs) const;
  bool operator!=(const Status& rhs) const;

  enum Code {
    kOk = 0,
    kNotFound = 1,
    kCorruption = 2,
    kNotSupported = 3,
    kInvalidArgument = 4,
    kIOError = 5,
    kMergeInProgress = 6,
    kIncomplete = 7,
    kShutdownInProgress = 8,
    kTimedOut = 9,
    kAborted = 10,
    kBusy = 11,
    kExpired = 12,
    kTryAgain = 13
  };

  Code code() const { return code_; }

  enum SubCode {
    kNone = 0,
    kMutexTimeout = 1,
    kLockTimeout = 2,
    kLockLimit = 3,
    kMaxSubCode
  };

  SubCode subcode() const { return subcode_; }

  // Return a success status.
  static Status OK() { return Status(); }

  // Return error status of an appropriate type.
  static Status NotFound(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kNotFound, msg, msg2);
  }
  // Fast path for not found without malloc;
  static Status NotFound(SubCode msg = kNone) { return Status(kNotFound, msg); }

  static Status Corruption(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kCorruption, msg, msg2);
  }
  static Status Corruption(SubCode msg = kNone) {
    return Status(kCorruption, msg);
  }

  static Status NotSupported(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kNotSupported, msg, msg2);
  }
  static Status NotSupported(SubCode msg = kNone) {
    return Status(kNotSupported, msg);
  }

  static Status InvalidArgument(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kInvalidArgument, msg, msg2);
  }
  static Status InvalidArgument(SubCode msg = kNone) {
    return Status(kInvalidArgument, msg);
  }

  static Status IOError(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kIOError, msg, msg2);
  }
  static Status IOError(SubCode msg = kNone) { return Status(kIOError, msg); }

  static Status MergeInProgress(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kMergeInProgress, msg, msg2);
  }
  static Status MergeInProgress(SubCode msg = kNone) {
    return Status(kMergeInProgress, msg);
  }

  static Status Incomplete(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kIncomplete, msg, msg2);
  }
  static Status Incomplete(SubCode msg = kNone) {
    return Status(kIncomplete, msg);
  }

  static Status ShutdownInProgress(SubCode msg = kNone) {
    return Status(kShutdownInProgress, msg);
  }
  static Status ShutdownInProgress(const Slice& msg,
                                   const Slice& msg2 = Slice()) {
    return Status(kShutdownInProgress, msg, msg2);
  }
  static Status Aborted(SubCode msg = kNone) { return Status(kAborted, msg); }
  static Status Aborted(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kAborted, msg, msg2);
  }

  static Status Busy(SubCode msg = kNone) { return Status(kBusy, msg); }
  static Status Busy(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kBusy, msg, msg2);
  }

  static Status TimedOut(SubCode msg = kNone) { return Status(kTimedOut, msg); }
  static Status TimedOut(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kTimedOut, msg, msg2);
  }

  static Status Expired(SubCode msg = kNone) { return Status(kExpired, msg); }
  static Status Expired(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kExpired, msg, msg2);
  }

  static Status TryAgain(SubCode msg = kNone) { return Status(kTryAgain, msg); }
  static Status TryAgain(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kTryAgain, msg, msg2);
  }

  // Returns true iff the status indicates success.
  bool ok() const { return code() == kOk; }

  // Returns true iff the status indicates a NotFound error.
  bool IsNotFound() const { return code() == kNotFound; }

  // Returns true iff the status indicates a Corruption error.
  bool IsCorruption() const { return code() == kCorruption; }

  // Returns true iff the status indicates a NotSupported error.
  bool IsNotSupported() const { return code() == kNotSupported; }

  // Returns true iff the status indicates an InvalidArgument error.
  bool IsInvalidArgument() const { return code() == kInvalidArgument; }

  // Returns true iff the status indicates an IOError.
  bool IsIOError() const { return code() == kIOError; }

  // Returns true iff the status indicates an MergeInProgress.
  bool IsMergeInProgress() const { return code() == kMergeInProgress; }

  // Returns true iff the status indicates Incomplete
  bool IsIncomplete() const { return code() == kIncomplete; }

  // Returns true iff the status indicates Shutdown In progress
  bool IsShutdownInProgress() const { return code() == kShutdownInProgress; }

  bool IsTimedOut() const { return code() == kTimedOut; }

  bool IsAborted() const { return code() == kAborted; }

  // Returns true iff the status indicates that a resource is Busy and
  // temporarily could not be acquired.
  bool IsBusy() const { return code() == kBusy; }

  // Returns true iff the status indicated that the operation has Expired.
  bool IsExpired() const { return code() == kExpired; }

  // Returns true iff the status indicates a TryAgain error.
  // This usually means that the operation failed, but may succeed if
  // re-attempted.
  bool IsTryAgain() const { return code() == kTryAgain; }

  // Return a string representation of this status suitable for printing.
  // Returns the string "OK" for success.
  std::string ToString() const;

 private:
  // A nullptr state_ (which is always the case for OK) means the message
  // is empty.
  // of the following form:
  //    state_[0..3] == length of message
  //    state_[4..]  == message
  Code code_;
  SubCode subcode_;
  const char* state_;

  static const char* msgs[static_cast<int>(kMaxSubCode)];

  explicit Status(Code _code, SubCode _subcode = kNone)
      : code_(_code), subcode_(_subcode), state_(nullptr) {}

  Status(Code _code, const Slice& msg, const Slice& msg2);
  static const char* CopyState(const char* s);
};

inline Status::Status(const Status& s) : code_(s.code_), subcode_(s.subcode_) {
  state_ = (s.state_ == nullptr) ? nullptr : CopyState(s.state_);
}
inline void Status::operator=(const Status& s) {
  // The following condition catches both aliasing (when this == &s),
  // and the common case where both s and *this are ok.
  code_ = s.code_;
  subcode_ = s.subcode_;
  if (state_ != s.state_) {
    delete[] state_;
    state_ = (s.state_ == nullptr) ? nullptr : CopyState(s.state_);
  }
}

inline bool Status::operator==(const Status& rhs) const {
  return (code_ == rhs.code_);
}

inline bool Status::operator!=(const Status& rhs) const {
  return !(*this == rhs);
}

}  // namespace rocksdb

#endif  // STORAGE_ROCKSDB_INCLUDE_STATUS_H_
#line 9 "/home/evan/source/rocksdb/include/rocksdb/table_properties.h"
#line 1 "/home/evan/source/rocksdb/include/rocksdb/types.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#ifndef STORAGE_ROCKSDB_INCLUDE_TYPES_H_
#define STORAGE_ROCKSDB_INCLUDE_TYPES_H_

#include <stdint.h>

namespace rocksdb {

// Define all public custom types here.

// Represents a sequence number in a WAL file.
typedef uint64_t SequenceNumber;

}  //  namespace rocksdb

#endif //  STORAGE_ROCKSDB_INCLUDE_TYPES_H_
#line 10 "/home/evan/source/rocksdb/include/rocksdb/table_properties.h"

namespace rocksdb {

// -- Table Properties
// Other than basic table properties, each table may also have the user
// collected properties.
// The value of the user-collected properties are encoded as raw bytes --
// users have to interprete these values by themselves.
// Note: To do prefix seek/scan in `UserCollectedProperties`, you can do
// something similar to:
//
// UserCollectedProperties props = ...;
// for (auto pos = props.lower_bound(prefix);
//      pos != props.end() && pos->first.compare(0, prefix.size(), prefix) == 0;
//      ++pos) {
//   ...
// }
typedef std::map<std::string, std::string> UserCollectedProperties;

// TableProperties contains a bunch of read-only properties of its associated
// table.
struct TableProperties {
 public:
  // the total size of all data blocks.
  uint64_t data_size = 0;
  // the size of index block.
  uint64_t index_size = 0;
  // the size of filter block.
  uint64_t filter_size = 0;
  // total raw key size
  uint64_t raw_key_size = 0;
  // total raw value size
  uint64_t raw_value_size = 0;
  // the number of blocks in this table
  uint64_t num_data_blocks = 0;
  // the number of entries in this table
  uint64_t num_entries = 0;
  // format version, reserved for backward compatibility
  uint64_t format_version = 0;
  // If 0, key is variable length. Otherwise number of bytes for each key.
  uint64_t fixed_key_len = 0;

  // The name of the filter policy used in this table.
  // If no filter policy is used, `filter_policy_name` will be an empty string.
  std::string filter_policy_name;

  // user collected properties
  UserCollectedProperties user_collected_properties;

  // convert this object to a human readable form
  //   @prop_delim: delimiter for each property.
  std::string ToString(const std::string& prop_delim = "; ",
                       const std::string& kv_delim = "=") const;

  // Aggregate the numerical member variables of the specified
  // TableProperties.
  void Add(const TableProperties& tp);
};

// table properties' human-readable names in the property block.
struct TablePropertiesNames {
  static const std::string kDataSize;
  static const std::string kIndexSize;
  static const std::string kFilterSize;
  static const std::string kRawKeySize;
  static const std::string kRawValueSize;
  static const std::string kNumDataBlocks;
  static const std::string kNumEntries;
  static const std::string kFormatVersion;
  static const std::string kFixedKeyLen;
  static const std::string kFilterPolicy;
};

extern const std::string kPropertiesBlock;

enum EntryType {
  kEntryPut,
  kEntryDelete,
  kEntrySingleDelete,
  kEntryMerge,
  kEntryOther,
};

// `TablePropertiesCollector` provides the mechanism for users to collect
// their own properties that they are interested in. This class is essentially
// a collection of callback functions that will be invoked during table
// building. It is construced with TablePropertiesCollectorFactory. The methods
// don't need to be thread-safe, as we will create exactly one
// TablePropertiesCollector object per table and then call it sequentially
class TablePropertiesCollector {
 public:
  virtual ~TablePropertiesCollector() {}

  // DEPRECATE User defined collector should implement AddUserKey(), though
  //           this old function still works for backward compatible reason.
  // Add() will be called when a new key/value pair is inserted into the table.
  // @params key    the user key that is inserted into the table.
  // @params value  the value that is inserted into the table.
  virtual Status Add(const Slice& key, const Slice& value) {
    return Status::InvalidArgument(
        "TablePropertiesCollector::Add() deprecated.");
  }

  // AddUserKey() will be called when a new key/value pair is inserted into the
  // table.
  // @params key    the user key that is inserted into the table.
  // @params value  the value that is inserted into the table.
  // @params file_size  file size up to now
  virtual Status AddUserKey(const Slice& key, const Slice& value,
                            EntryType type, SequenceNumber seq,
                            uint64_t file_size) {
    // For backwards-compatibility.
    return Add(key, value);
  }

  // Finish() will be called when a table has already been built and is ready
  // for writing the properties block.
  // @params properties  User will add their collected statistics to
  // `properties`.
  virtual Status Finish(UserCollectedProperties* properties) = 0;

  // Return the human-readable properties, where the key is property name and
  // the value is the human-readable form of value.
  virtual UserCollectedProperties GetReadableProperties() const = 0;

  // The name of the properties collector can be used for debugging purpose.
  virtual const char* Name() const = 0;

  // EXPERIMENTAL Return whether the output file should be further compacted
  virtual bool NeedCompact() const { return false; }
};

// Constructs TablePropertiesCollector. Internals create a new
// TablePropertiesCollector for each new table
class TablePropertiesCollectorFactory {
 public:
  virtual ~TablePropertiesCollectorFactory() {}
  // has to be thread-safe
  virtual TablePropertiesCollector* CreateTablePropertiesCollector() = 0;

  // The name of the properties collector can be used for debugging purpose.
  virtual const char* Name() const = 0;
};

// Extra properties
// Below is a list of non-basic properties that are collected by database
// itself. Especially some properties regarding to the internal keys (which
// is unknown to `table`).
extern uint64_t GetDeletedKeys(const UserCollectedProperties& props);

}  // namespace rocksdb
#line 1 "/home/evan/source/rocksdb/include/rocksdb/comparator.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_ROCKSDB_INCLUDE_COMPARATOR_H_
#define STORAGE_ROCKSDB_INCLUDE_COMPARATOR_H_

#include <string>

namespace rocksdb {

class Slice;

// A Comparator object provides a total order across slices that are
// used as keys in an sstable or a database.  A Comparator implementation
// must be thread-safe since rocksdb may invoke its methods concurrently
// from multiple threads.
class Comparator {
 public:
  virtual ~Comparator();

  // Three-way comparison.  Returns value:
  //   < 0 iff "a" < "b",
  //   == 0 iff "a" == "b",
  //   > 0 iff "a" > "b"
  virtual int Compare(const Slice& a, const Slice& b) const = 0;

  // Compares two slices for equality. The following invariant should always
  // hold (and is the default implementation):
  //   Equal(a, b) iff Compare(a, b) == 0
  // Overwrite only if equality comparisons can be done more efficiently than
  // three-way comparisons.
  virtual bool Equal(const Slice& a, const Slice& b) const {
    return Compare(a, b) == 0;
  }

  // The name of the comparator.  Used to check for comparator
  // mismatches (i.e., a DB created with one comparator is
  // accessed using a different comparator.
  //
  // The client of this package should switch to a new name whenever
  // the comparator implementation changes in a way that will cause
  // the relative ordering of any two keys to change.
  //
  // Names starting with "rocksdb." are reserved and should not be used
  // by any clients of this package.
  virtual const char* Name() const = 0;

  // Advanced functions: these are used to reduce the space requirements
  // for internal data structures like index blocks.

  // If *start < limit, changes *start to a short string in [start,limit).
  // Simple comparator implementations may return with *start unchanged,
  // i.e., an implementation of this method that does nothing is correct.
  virtual void FindShortestSeparator(
      std::string* start,
      const Slice& limit) const = 0;

  // Changes *key to a short string >= *key.
  // Simple comparator implementations may return with *key unchanged,
  // i.e., an implementation of this method that does nothing is correct.
  virtual void FindShortSuccessor(std::string* key) const = 0;
};

// Return a builtin comparator that uses lexicographic byte-wise
// ordering.  The result remains the property of this module and
// must not be deleted.
extern const Comparator* BytewiseComparator();

// Return a builtin comparator that uses reverse lexicographic byte-wise
// ordering.
extern const Comparator* ReverseBytewiseComparator();

}  // namespace rocksdb

#endif  // STORAGE_ROCKSDB_INCLUDE_COMPARATOR_H_
#line 1 "/home/evan/source/rocksdb/include/rocksdb/env.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// An Env is an interface used by the rocksdb implementation to access
// operating system functionality like the filesystem etc.  Callers
// may wish to provide a custom Env object when opening a database to
// get fine gain control; e.g., to rate limit file system operations.
//
// All Env implementations are safe for concurrent access from
// multiple threads without any external synchronization.

#ifndef STORAGE_ROCKSDB_INCLUDE_ENV_H_
#define STORAGE_ROCKSDB_INCLUDE_ENV_H_

#include <stdint.h>
#include <cstdarg>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#line 1 "/home/evan/source/rocksdb/include/rocksdb/thread_status.h"
// Copyright (c) 2014, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
//
// This file defines the structures for exposing run-time status of any
// rocksdb-related thread.  Such run-time status can be obtained via
// GetThreadList() API.
//
// Note that all thread-status features are still under-development, and
// thus APIs and class definitions might subject to change at this point.
// Will remove this comment once the APIs have been finalized.


#include <stdint.h>
#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>

#ifndef ROCKSDB_USING_THREAD_STATUS
#define ROCKSDB_USING_THREAD_STATUS \
    !defined(ROCKSDB_LITE) && \
    !defined(NROCKSDB_THREAD_STATUS) && \
    !defined(OS_MACOSX) && \
    !defined(IOS_CROSS_COMPILE)
#endif

namespace rocksdb {

// TODO(yhchiang): remove this function once c++14 is available
//                 as std::max will be able to cover this.
// Current MS compiler does not support constexpr
template <int A, int B>
struct constexpr_max {
  static const int result = (A > B) ? A : B;
};

// A structure that describes the current status of a thread.
// The status of active threads can be fetched using
// rocksdb::GetThreadList().
struct ThreadStatus {
  // The type of a thread.
  enum ThreadType : int {
    HIGH_PRIORITY = 0,  // RocksDB BG thread in high-pri thread pool
    LOW_PRIORITY,  // RocksDB BG thread in low-pri thread pool
    USER,  // User thread (Non-RocksDB BG thread)
    NUM_THREAD_TYPES
  };

  // The type used to refer to a thread operation.
  // A thread operation describes high-level action of a thread.
  // Examples include compaction and flush.
  enum OperationType : int {
    OP_UNKNOWN = 0,
    OP_COMPACTION,
    OP_FLUSH,
    NUM_OP_TYPES
  };

  enum OperationStage : int {
    STAGE_UNKNOWN = 0,
    STAGE_FLUSH_RUN,
    STAGE_FLUSH_WRITE_L0,
    STAGE_COMPACTION_PREPARE,
    STAGE_COMPACTION_RUN,
    STAGE_COMPACTION_PROCESS_KV,
    STAGE_COMPACTION_INSTALL,
    STAGE_COMPACTION_SYNC_FILE,
    STAGE_PICK_MEMTABLES_TO_FLUSH,
    STAGE_MEMTABLE_ROLLBACK,
    STAGE_MEMTABLE_INSTALL_FLUSH_RESULTS,
    NUM_OP_STAGES
  };

  enum CompactionPropertyType : int {
    COMPACTION_JOB_ID = 0,
    COMPACTION_INPUT_OUTPUT_LEVEL,
    COMPACTION_PROP_FLAGS,
    COMPACTION_TOTAL_INPUT_BYTES,
    COMPACTION_BYTES_READ,
    COMPACTION_BYTES_WRITTEN,
    NUM_COMPACTION_PROPERTIES
  };

  enum FlushPropertyType : int {
    FLUSH_JOB_ID = 0,
    FLUSH_BYTES_MEMTABLES,
    FLUSH_BYTES_WRITTEN,
    NUM_FLUSH_PROPERTIES
  };

  // The maximum number of properties of an operation.
  // This number should be set to the biggest NUM_XXX_PROPERTIES.
  static const int kNumOperationProperties =
      constexpr_max<NUM_COMPACTION_PROPERTIES, NUM_FLUSH_PROPERTIES>::result;

  // The type used to refer to a thread state.
  // A state describes lower-level action of a thread
  // such as reading / writing a file or waiting for a mutex.
  enum StateType : int {
    STATE_UNKNOWN = 0,
    STATE_MUTEX_WAIT = 1,
    NUM_STATE_TYPES
  };

  ThreadStatus(const uint64_t _id,
               const ThreadType _thread_type,
               const std::string& _db_name,
               const std::string& _cf_name,
               const OperationType _operation_type,
               const uint64_t _op_elapsed_micros,
               const OperationStage _operation_stage,
               const uint64_t _op_props[],
               const StateType _state_type) :
      thread_id(_id), thread_type(_thread_type),
      db_name(_db_name),
      cf_name(_cf_name),
      operation_type(_operation_type),
      op_elapsed_micros(_op_elapsed_micros),
      operation_stage(_operation_stage),
      state_type(_state_type) {
    for (int i = 0; i < kNumOperationProperties; ++i) {
      op_properties[i] = _op_props[i];
    }
  }

  // An unique ID for the thread.
  const uint64_t thread_id;

  // The type of the thread, it could be HIGH_PRIORITY,
  // LOW_PRIORITY, and USER
  const ThreadType thread_type;

  // The name of the DB instance where the thread is currently
  // involved with.  It would be set to empty string if the thread
  // does not involve in any DB operation.
  const std::string db_name;

  // The name of the column family where the thread is currently
  // It would be set to empty string if the thread does not involve
  // in any column family.
  const std::string cf_name;

  // The operation (high-level action) that the current thread is involved.
  const OperationType operation_type;

  // The elapsed time in micros of the current thread operation.
  const uint64_t op_elapsed_micros;

  // An integer showing the current stage where the thread is involved
  // in the current operation.
  const OperationStage operation_stage;

  // A list of properties that describe some details about the current
  // operation.  Same field in op_properties[] might have different
  // meanings for different operations.
  uint64_t op_properties[kNumOperationProperties];

  // The state (lower-level action) that the current thread is involved.
  const StateType state_type;

  // The followings are a set of utility functions for interpreting
  // the information of ThreadStatus

  static const std::string& GetThreadTypeName(ThreadType thread_type);

  // Obtain the name of an operation given its type.
  static const std::string& GetOperationName(OperationType op_type);

  static const std::string MicrosToString(uint64_t op_elapsed_time);

  // Obtain a human-readable string describing the specified operation stage.
  static const std::string& GetOperationStageName(
      OperationStage stage);

  // Obtain the name of the "i"th operation property of the
  // specified operation.
  static const std::string& GetOperationPropertyName(
      OperationType op_type, int i);

  // Translate the "i"th property of the specified operation given
  // a property value.
  static std::map<std::string, uint64_t>
      InterpretOperationProperties(
          OperationType op_type, const uint64_t* op_properties);

  // Obtain the name of a state given its type.
  static const std::string& GetStateName(StateType state_type);
};


}  // namespace rocksdb
#line 27 "/home/evan/source/rocksdb/include/rocksdb/env.h"

#ifdef _WIN32
// Windows API macro interference
#undef DeleteFile
#undef GetCurrentTime
#endif

namespace rocksdb {

class FileLock;
class Logger;
class RandomAccessFile;
class SequentialFile;
class Slice;
class WritableFile;
class Directory;
struct DBOptions;
class RateLimiter;
class ThreadStatusUpdater;
struct ThreadStatus;

using std::unique_ptr;
using std::shared_ptr;


// Options while opening a file to read/write
struct EnvOptions {

  // construct with default Options
  EnvOptions();

  // construct from Options
  explicit EnvOptions(const DBOptions& options);

  // If true, then allow caching of data in environment buffers
  bool use_os_buffer = true;

   // If true, then use mmap to read data
  bool use_mmap_reads = false;

   // If true, then use mmap to write data
  bool use_mmap_writes = true;

  // If false, fallocate() calls are bypassed
  bool allow_fallocate = true;

  // If true, set the FD_CLOEXEC on open fd.
  bool set_fd_cloexec = true;

  // Allows OS to incrementally sync files to disk while they are being
  // written, in the background. Issue one request for every bytes_per_sync
  // written. 0 turns it off.
  // Default: 0
  uint64_t bytes_per_sync = 0;

  // If true, we will preallocate the file with FALLOC_FL_KEEP_SIZE flag, which
  // means that file size won't change as part of preallocation.
  // If false, preallocation will also change the file size. This option will
  // improve the performance in workloads where you sync the data on every
  // write. By default, we set it to true for MANIFEST writes and false for
  // WAL writes
  bool fallocate_with_keep_size = true;

  // If not nullptr, write rate limiting is enabled for flush and compaction
  RateLimiter* rate_limiter = nullptr;
};

class Env {
 public:
  Env() : thread_status_updater_(nullptr) {}

  virtual ~Env();

  // Return a default environment suitable for the current operating
  // system.  Sophisticated users may wish to provide their own Env
  // implementation instead of relying on this default environment.
  //
  // The result of Default() belongs to rocksdb and must never be deleted.
  static Env* Default();

  // Create a brand new sequentially-readable file with the specified name.
  // On success, stores a pointer to the new file in *result and returns OK.
  // On failure stores nullptr in *result and returns non-OK.  If the file does
  // not exist, returns a non-OK status.
  //
  // The returned file will only be accessed by one thread at a time.
  virtual Status NewSequentialFile(const std::string& fname,
                                   unique_ptr<SequentialFile>* result,
                                   const EnvOptions& options)
                                   = 0;

  // Create a brand new random access read-only file with the
  // specified name.  On success, stores a pointer to the new file in
  // *result and returns OK.  On failure stores nullptr in *result and
  // returns non-OK.  If the file does not exist, returns a non-OK
  // status.
  //
  // The returned file may be concurrently accessed by multiple threads.
  virtual Status NewRandomAccessFile(const std::string& fname,
                                     unique_ptr<RandomAccessFile>* result,
                                     const EnvOptions& options)
                                     = 0;

  // Create an object that writes to a new file with the specified
  // name.  Deletes any existing file with the same name and creates a
  // new file.  On success, stores a pointer to the new file in
  // *result and returns OK.  On failure stores nullptr in *result and
  // returns non-OK.
  //
  // The returned file will only be accessed by one thread at a time.
  virtual Status NewWritableFile(const std::string& fname,
                                 unique_ptr<WritableFile>* result,
                                 const EnvOptions& options) = 0;

  // Create an object that represents a directory. Will fail if directory
  // doesn't exist. If the directory exists, it will open the directory
  // and create a new Directory object.
  //
  // On success, stores a pointer to the new Directory in
  // *result and returns OK. On failure stores nullptr in *result and
  // returns non-OK.
  virtual Status NewDirectory(const std::string& name,
                              unique_ptr<Directory>* result) = 0;

  // Returns OK if the named file exists.
  //         NotFound if the named file does not exist,
  //                  the calling process does not have permission to determine
  //                  whether this file exists, or if the path is invalid.
  //         IOError if an IO Error was encountered
  virtual Status FileExists(const std::string& fname) = 0;

  // Store in *result the names of the children of the specified directory.
  // The names are relative to "dir".
  // Original contents of *results are dropped.
  virtual Status GetChildren(const std::string& dir,
                             std::vector<std::string>* result) = 0;

  // Delete the named file.
  virtual Status DeleteFile(const std::string& fname) = 0;

  // Create the specified directory. Returns error if directory exists.
  virtual Status CreateDir(const std::string& dirname) = 0;

  // Creates directory if missing. Return Ok if it exists, or successful in
  // Creating.
  virtual Status CreateDirIfMissing(const std::string& dirname) = 0;

  // Delete the specified directory.
  virtual Status DeleteDir(const std::string& dirname) = 0;

  // Store the size of fname in *file_size.
  virtual Status GetFileSize(const std::string& fname, uint64_t* file_size) = 0;

  // Store the last modification time of fname in *file_mtime.
  virtual Status GetFileModificationTime(const std::string& fname,
                                         uint64_t* file_mtime) = 0;
  // Rename file src to target.
  virtual Status RenameFile(const std::string& src,
                            const std::string& target) = 0;

  // Hard Link file src to target.
  virtual Status LinkFile(const std::string& src, const std::string& target) {
    return Status::NotSupported("LinkFile is not supported for this Env");
  }

  // Lock the specified file.  Used to prevent concurrent access to
  // the same db by multiple processes.  On failure, stores nullptr in
  // *lock and returns non-OK.
  //
  // On success, stores a pointer to the object that represents the
  // acquired lock in *lock and returns OK.  The caller should call
  // UnlockFile(*lock) to release the lock.  If the process exits,
  // the lock will be automatically released.
  //
  // If somebody else already holds the lock, finishes immediately
  // with a failure.  I.e., this call does not wait for existing locks
  // to go away.
  //
  // May create the named file if it does not already exist.
  virtual Status LockFile(const std::string& fname, FileLock** lock) = 0;

  // Release the lock acquired by a previous successful call to LockFile.
  // REQUIRES: lock was returned by a successful LockFile() call
  // REQUIRES: lock has not already been unlocked.
  virtual Status UnlockFile(FileLock* lock) = 0;

  // Priority for scheduling job in thread pool
  enum Priority { LOW, HIGH, TOTAL };

  // Priority for requesting bytes in rate limiter scheduler
  enum IOPriority {
    IO_LOW = 0,
    IO_HIGH = 1,
    IO_TOTAL = 2
  };

  // Arrange to run "(*function)(arg)" once in a background thread, in
  // the thread pool specified by pri. By default, jobs go to the 'LOW'
  // priority thread pool.

  // "function" may run in an unspecified thread.  Multiple functions
  // added to the same Env may run concurrently in different threads.
  // I.e., the caller may not assume that background work items are
  // serialized.
  virtual void Schedule(void (*function)(void* arg), void* arg,
                        Priority pri = LOW, void* tag = nullptr) = 0;

  // Arrange to remove jobs for given arg from the queue_ if they are not
  // already scheduled. Caller is expected to have exclusive lock on arg.
  virtual int UnSchedule(void* arg, Priority pri) { return 0; }

  // Start a new thread, invoking "function(arg)" within the new thread.
  // When "function(arg)" returns, the thread will be destroyed.
  virtual void StartThread(void (*function)(void* arg), void* arg) = 0;

  // Wait for all threads started by StartThread to terminate.
  virtual void WaitForJoin() {}

  // Get thread pool queue length for specific thrad pool.
  virtual unsigned int GetThreadPoolQueueLen(Priority pri = LOW) const {
    return 0;
  }

  // *path is set to a temporary directory that can be used for testing. It may
  // or many not have just been created. The directory may or may not differ
  // between runs of the same process, but subsequent calls will return the
  // same directory.
  virtual Status GetTestDirectory(std::string* path) = 0;

  // Create and return a log file for storing informational messages.
  virtual Status NewLogger(const std::string& fname,
                           shared_ptr<Logger>* result) = 0;

  // Returns the number of micro-seconds since some fixed point in time. Only
  // useful for computing deltas of time.
  // However, it is often used as system time such as in GenericRateLimiter
  // and other places so a port needs to return system time in order to work.
  virtual uint64_t NowMicros() = 0;

  // Returns the number of nano-seconds since some fixed point in time. Only
  // useful for computing deltas of time in one run.
  // Default implementation simply relies on NowMicros
  virtual uint64_t NowNanos() {
    return NowMicros() * 1000;
  }

  // Sleep/delay the thread for the perscribed number of micro-seconds.
  virtual void SleepForMicroseconds(int micros) = 0;

  // Get the current host name.
  virtual Status GetHostName(char* name, uint64_t len) = 0;

  // Get the number of seconds since the Epoch, 1970-01-01 00:00:00 (UTC).
  virtual Status GetCurrentTime(int64_t* unix_time) = 0;

  // Get full directory name for this db.
  virtual Status GetAbsolutePath(const std::string& db_path,
      std::string* output_path) = 0;

  // The number of background worker threads of a specific thread pool
  // for this environment. 'LOW' is the default pool.
  // default number: 1
  virtual void SetBackgroundThreads(int number, Priority pri = LOW) = 0;

  // Enlarge number of background worker threads of a specific thread pool
  // for this environment if it is smaller than specified. 'LOW' is the default
  // pool.
  virtual void IncBackgroundThreadsIfNeeded(int number, Priority pri) = 0;

  // Lower IO priority for threads from the specified pool.
  virtual void LowerThreadPoolIOPriority(Priority pool = LOW) {}

  // Converts seconds-since-Jan-01-1970 to a printable string
  virtual std::string TimeToString(uint64_t time) = 0;

  // Generates a unique id that can be used to identify a db
  virtual std::string GenerateUniqueId();

  // OptimizeForLogWrite will create a new EnvOptions object that is a copy of
  // the EnvOptions in the parameters, but is optimized for writing log files.
  // Default implementation returns the copy of the same object.
  virtual EnvOptions OptimizeForLogWrite(const EnvOptions& env_options,
                                         const DBOptions& db_options) const;
  // OptimizeForManifestWrite will create a new EnvOptions object that is a copy
  // of the EnvOptions in the parameters, but is optimized for writing manifest
  // files. Default implementation returns the copy of the same object.
  virtual EnvOptions OptimizeForManifestWrite(const EnvOptions& env_options)
      const;

  // Returns the status of all threads that belong to the current Env.
  virtual Status GetThreadList(std::vector<ThreadStatus>* thread_list) {
    return Status::NotSupported("Not supported.");
  }

  // Returns the pointer to ThreadStatusUpdater.  This function will be
  // used in RocksDB internally to update thread status and supports
  // GetThreadList().
  virtual ThreadStatusUpdater* GetThreadStatusUpdater() const {
    return thread_status_updater_;
  }

  // Returns the ID of the current thread.
  virtual uint64_t GetThreadID() const;

 protected:
  // The pointer to an internal structure that will update the
  // status of each thread.
  ThreadStatusUpdater* thread_status_updater_;

 private:
  // No copying allowed
  Env(const Env&);
  void operator=(const Env&);
};

// The factory function to construct a ThreadStatusUpdater.  Any Env
// that supports GetThreadList() feature should call this function in its
// constructor to initialize thread_status_updater_.
ThreadStatusUpdater* CreateThreadStatusUpdater();

// A file abstraction for reading sequentially through a file
class SequentialFile {
 public:
  SequentialFile() { }
  virtual ~SequentialFile();

  // Read up to "n" bytes from the file.  "scratch[0..n-1]" may be
  // written by this routine.  Sets "*result" to the data that was
  // read (including if fewer than "n" bytes were successfully read).
  // May set "*result" to point at data in "scratch[0..n-1]", so
  // "scratch[0..n-1]" must be live when "*result" is used.
  // If an error was encountered, returns a non-OK status.
  //
  // REQUIRES: External synchronization
  virtual Status Read(size_t n, Slice* result, char* scratch) = 0;

  // Skip "n" bytes from the file. This is guaranteed to be no
  // slower that reading the same data, but may be faster.
  //
  // If end of file is reached, skipping will stop at the end of the
  // file, and Skip will return OK.
  //
  // REQUIRES: External synchronization
  virtual Status Skip(uint64_t n) = 0;

  // Remove any kind of caching of data from the offset to offset+length
  // of this file. If the length is 0, then it refers to the end of file.
  // If the system is not caching the file contents, then this is a noop.
  virtual Status InvalidateCache(size_t offset, size_t length) {
    return Status::NotSupported("InvalidateCache not supported.");
  }
};

// A file abstraction for randomly reading the contents of a file.
class RandomAccessFile {
 public:
  RandomAccessFile() { }
  virtual ~RandomAccessFile();

  // Read up to "n" bytes from the file starting at "offset".
  // "scratch[0..n-1]" may be written by this routine.  Sets "*result"
  // to the data that was read (including if fewer than "n" bytes were
  // successfully read).  May set "*result" to point at data in
  // "scratch[0..n-1]", so "scratch[0..n-1]" must be live when
  // "*result" is used.  If an error was encountered, returns a non-OK
  // status.
  //
  // Safe for concurrent use by multiple threads.
  virtual Status Read(uint64_t offset, size_t n, Slice* result,
                      char* scratch) const = 0;

  // Used by the file_reader_writer to decide if the ReadAhead wrapper
  // should simply forward the call and do not enact buffering or locking.
  virtual bool ShouldForwardRawRequest() const {
    return false;
  }

  // Tries to get an unique ID for this file that will be the same each time
  // the file is opened (and will stay the same while the file is open).
  // Furthermore, it tries to make this ID at most "max_size" bytes. If such an
  // ID can be created this function returns the length of the ID and places it
  // in "id"; otherwise, this function returns 0, in which case "id"
  // may not have been modified.
  //
  // This function guarantees, for IDs from a given environment, two unique ids
  // cannot be made equal to eachother by adding arbitrary bytes to one of
  // them. That is, no unique ID is the prefix of another.
  //
  // This function guarantees that the returned ID will not be interpretable as
  // a single varint.
  //
  // Note: these IDs are only valid for the duration of the process.
  virtual size_t GetUniqueId(char* id, size_t max_size) const {
    return 0; // Default implementation to prevent issues with backwards
              // compatibility.
  };

  enum AccessPattern { NORMAL, RANDOM, SEQUENTIAL, WILLNEED, DONTNEED };

  virtual void Hint(AccessPattern pattern) {}

  // Remove any kind of caching of data from the offset to offset+length
  // of this file. If the length is 0, then it refers to the end of file.
  // If the system is not caching the file contents, then this is a noop.
  virtual Status InvalidateCache(size_t offset, size_t length) {
    return Status::NotSupported("InvalidateCache not supported.");
  }
};

// A file abstraction for sequential writing.  The implementation
// must provide buffering since callers may append small fragments
// at a time to the file.
class WritableFile {
 public:
  WritableFile()
    : last_preallocated_block_(0),
      preallocation_block_size_(0),
      io_priority_(Env::IO_TOTAL) {
  }
  virtual ~WritableFile();

  // Indicates if the class makes use of unbuffered I/O
  virtual bool UseOSBuffer() const {
    return true;
  }

  const size_t c_DefaultPageSize = 4 * 1024;

  // This is needed when you want to allocate
  // AlignedBuffer for use with file I/O classes
  // Used for unbuffered file I/O when UseOSBuffer() returns false
  virtual size_t GetRequiredBufferAlignment() const {
    return c_DefaultPageSize;
  }

  virtual Status Append(const Slice& data) = 0;

  // Positioned write for unbuffered access default forward
  // to simple append as most of the tests are buffered by default
  virtual Status PositionedAppend(const Slice& /* data */, uint64_t /* offset */) {
    return Status::NotSupported();
  }

  // Truncate is necessary to trim the file to the correct size
  // before closing. It is not always possible to keep track of the file
  // size due to whole pages writes. The behavior is undefined if called
  // with other writes to follow.
  virtual Status Truncate(uint64_t size) {
    return Status::OK();
  }
  virtual Status Close() = 0;
  virtual Status Flush() = 0;
  virtual Status Sync() = 0; // sync data

  /*
   * Sync data and/or metadata as well.
   * By default, sync only data.
   * Override this method for environments where we need to sync
   * metadata as well.
   */
  virtual Status Fsync() {
    return Sync();
  }

  // true if Sync() and Fsync() are safe to call concurrently with Append()
  // and Flush().
  virtual bool IsSyncThreadSafe() const {
    return false;
  }

  // Indicates the upper layers if the current WritableFile implementation
  // uses direct IO.
  virtual bool UseDirectIO() const { return false; }

  /*
   * Change the priority in rate limiter if rate limiting is enabled.
   * If rate limiting is not enabled, this call has no effect.
   */
  virtual void SetIOPriority(Env::IOPriority pri) {
    io_priority_ = pri;
  }

  virtual Env::IOPriority GetIOPriority() { return io_priority_; }

  /*
   * Get the size of valid data in the file.
   */
  virtual uint64_t GetFileSize() {
    return 0;
  }

  /*
   * Get and set the default pre-allocation block size for writes to
   * this file.  If non-zero, then Allocate will be used to extend the
   * underlying storage of a file (generally via fallocate) if the Env
   * instance supports it.
   */
  void SetPreallocationBlockSize(size_t size) {
    preallocation_block_size_ = size;
  }

  virtual void GetPreallocationStatus(size_t* block_size,
                                      size_t* last_allocated_block) {
    *last_allocated_block = last_preallocated_block_;
    *block_size = preallocation_block_size_;
  }

  // For documentation, refer to RandomAccessFile::GetUniqueId()
  virtual size_t GetUniqueId(char* id, size_t max_size) const {
    return 0; // Default implementation to prevent issues with backwards
  }

  // Remove any kind of caching of data from the offset to offset+length
  // of this file. If the length is 0, then it refers to the end of file.
  // If the system is not caching the file contents, then this is a noop.
  // This call has no effect on dirty pages in the cache.
  virtual Status InvalidateCache(size_t offset, size_t length) {
    return Status::NotSupported("InvalidateCache not supported.");
  }

  // Sync a file range with disk.
  // offset is the starting byte of the file range to be synchronized.
  // nbytes specifies the length of the range to be synchronized.
  // This asks the OS to initiate flushing the cached data to disk,
  // without waiting for completion.
  // Default implementation does nothing.
  virtual Status RangeSync(off_t offset, off_t nbytes) { return Status::OK(); }

  // PrepareWrite performs any necessary preparation for a write
  // before the write actually occurs.  This allows for pre-allocation
  // of space on devices where it can result in less file
  // fragmentation and/or less waste from over-zealous filesystem
  // pre-allocation.
  void PrepareWrite(size_t offset, size_t len) {
    if (preallocation_block_size_ == 0) {
      return;
    }
    // If this write would cross one or more preallocation blocks,
    // determine what the last preallocation block necesessary to
    // cover this write would be and Allocate to that point.
    const auto block_size = preallocation_block_size_;
    size_t new_last_preallocated_block =
      (offset + len + block_size - 1) / block_size;
    if (new_last_preallocated_block > last_preallocated_block_) {
      size_t num_spanned_blocks =
        new_last_preallocated_block - last_preallocated_block_;
      Allocate(static_cast<off_t>(block_size * last_preallocated_block_),
               static_cast<off_t>(block_size * num_spanned_blocks));
      last_preallocated_block_ = new_last_preallocated_block;
    }
  }

 protected:
  /*
   * Pre-allocate space for a file.
   */
  virtual Status Allocate(off_t offset, off_t len) {
    return Status::OK();
  }

  size_t preallocation_block_size() { return preallocation_block_size_; }

 private:
  size_t last_preallocated_block_;
  size_t preallocation_block_size_;
  // No copying allowed
  WritableFile(const WritableFile&);
  void operator=(const WritableFile&);

 protected:
  friend class WritableFileWrapper;

  Env::IOPriority io_priority_;
};

// Directory object represents collection of files and implements
// filesystem operations that can be executed on directories.
class Directory {
 public:
  virtual ~Directory() {}
  // Fsync directory. Can be called concurrently from multiple threads.
  virtual Status Fsync() = 0;
};

enum InfoLogLevel : unsigned char {
  DEBUG_LEVEL = 0,
  INFO_LEVEL,
  WARN_LEVEL,
  ERROR_LEVEL,
  FATAL_LEVEL,
  HEADER_LEVEL,
  NUM_INFO_LOG_LEVELS,
};

// An interface for writing log messages.
class Logger {
 public:
  size_t kDoNotSupportGetLogFileSize = std::numeric_limits<size_t>::max();

  explicit Logger(const InfoLogLevel log_level = InfoLogLevel::INFO_LEVEL)
      : log_level_(log_level) {}
  virtual ~Logger();

  // Write a header to the log file with the specified format
  // It is recommended that you log all header information at the start of the
  // application. But it is not enforced.
  virtual void LogHeader(const char* format, va_list ap) {
    // Default implementation does a simple INFO level log write.
    // Please override as per the logger class requirement.
    Logv(format, ap);
  }

  // Write an entry to the log file with the specified format.
  virtual void Logv(const char* format, va_list ap) = 0;

  // Write an entry to the log file with the specified log level
  // and format.  Any log with level under the internal log level
  // of *this (see @SetInfoLogLevel and @GetInfoLogLevel) will not be
  // printed.
  virtual void Logv(const InfoLogLevel log_level, const char* format, va_list ap);

  virtual size_t GetLogFileSize() const { return kDoNotSupportGetLogFileSize; }
  // Flush to the OS buffers
  virtual void Flush() {}
  virtual InfoLogLevel GetInfoLogLevel() const { return log_level_; }
  virtual void SetInfoLogLevel(const InfoLogLevel log_level) {
    log_level_ = log_level;
  }

 private:
  // No copying allowed
  Logger(const Logger&);
  void operator=(const Logger&);
  InfoLogLevel log_level_;
};


// Identifies a locked file.
class FileLock {
 public:
  FileLock() { }
  virtual ~FileLock();
 private:
  // No copying allowed
  FileLock(const FileLock&);
  void operator=(const FileLock&);
};

extern void LogFlush(const shared_ptr<Logger>& info_log);

extern void Log(const InfoLogLevel log_level,
                const shared_ptr<Logger>& info_log, const char* format, ...);

// a set of log functions with different log levels.
extern void Header(const shared_ptr<Logger>& info_log, const char* format, ...);
extern void Debug(const shared_ptr<Logger>& info_log, const char* format, ...);
extern void Info(const shared_ptr<Logger>& info_log, const char* format, ...);
extern void Warn(const shared_ptr<Logger>& info_log, const char* format, ...);
extern void Error(const shared_ptr<Logger>& info_log, const char* format, ...);
extern void Fatal(const shared_ptr<Logger>& info_log, const char* format, ...);

// Log the specified data to *info_log if info_log is non-nullptr.
// The default info log level is InfoLogLevel::ERROR.
extern void Log(const shared_ptr<Logger>& info_log, const char* format, ...)
#   if defined(__GNUC__) || defined(__clang__)
    __attribute__((__format__ (__printf__, 2, 3)))
#   endif
    ;

extern void LogFlush(Logger *info_log);

extern void Log(const InfoLogLevel log_level, Logger* info_log,
                const char* format, ...);

// The default info log level is InfoLogLevel::ERROR.
extern void Log(Logger* info_log, const char* format, ...)
#   if defined(__GNUC__) || defined(__clang__)
    __attribute__((__format__ (__printf__, 2, 3)))
#   endif
    ;

// a set of log functions with different log levels.
extern void Header(Logger* info_log, const char* format, ...);
extern void Debug(Logger* info_log, const char* format, ...);
extern void Info(Logger* info_log, const char* format, ...);
extern void Warn(Logger* info_log, const char* format, ...);
extern void Error(Logger* info_log, const char* format, ...);
extern void Fatal(Logger* info_log, const char* format, ...);

// A utility routine: write "data" to the named file.
extern Status WriteStringToFile(Env* env, const Slice& data,
                                const std::string& fname,
                                bool should_sync = false);

// A utility routine: read contents of named file into *data
extern Status ReadFileToString(Env* env, const std::string& fname,
                               std::string* data);

// An implementation of Env that forwards all calls to another Env.
// May be useful to clients who wish to override just part of the
// functionality of another Env.
class EnvWrapper : public Env {
 public:
  // Initialize an EnvWrapper that delegates all calls to *t
  explicit EnvWrapper(Env* t) : target_(t) { }
  virtual ~EnvWrapper();

  // Return the target to which this Env forwards all calls
  Env* target() const { return target_; }

  // The following text is boilerplate that forwards all methods to target()
  Status NewSequentialFile(const std::string& f, unique_ptr<SequentialFile>* r,
                           const EnvOptions& options) override {
    return target_->NewSequentialFile(f, r, options);
  }
  Status NewRandomAccessFile(const std::string& f,
                             unique_ptr<RandomAccessFile>* r,
                             const EnvOptions& options) override {
    return target_->NewRandomAccessFile(f, r, options);
  }
  Status NewWritableFile(const std::string& f, unique_ptr<WritableFile>* r,
                         const EnvOptions& options) override {
    return target_->NewWritableFile(f, r, options);
  }
  virtual Status NewDirectory(const std::string& name,
                              unique_ptr<Directory>* result) override {
    return target_->NewDirectory(name, result);
  }
  Status FileExists(const std::string& f) override {
    return target_->FileExists(f);
  }
  Status GetChildren(const std::string& dir,
                     std::vector<std::string>* r) override {
    return target_->GetChildren(dir, r);
  }
  Status DeleteFile(const std::string& f) override {
    return target_->DeleteFile(f);
  }
  Status CreateDir(const std::string& d) override {
    return target_->CreateDir(d);
  }
  Status CreateDirIfMissing(const std::string& d) override {
    return target_->CreateDirIfMissing(d);
  }
  Status DeleteDir(const std::string& d) override {
    return target_->DeleteDir(d);
  }
  Status GetFileSize(const std::string& f, uint64_t* s) override {
    return target_->GetFileSize(f, s);
  }

  Status GetFileModificationTime(const std::string& fname,
                                 uint64_t* file_mtime) override {
    return target_->GetFileModificationTime(fname, file_mtime);
  }

  Status RenameFile(const std::string& s, const std::string& t) override {
    return target_->RenameFile(s, t);
  }

  Status LinkFile(const std::string& s, const std::string& t) override {
    return target_->LinkFile(s, t);
  }

  Status LockFile(const std::string& f, FileLock** l) override {
    return target_->LockFile(f, l);
  }

  Status UnlockFile(FileLock* l) override { return target_->UnlockFile(l); }

  void Schedule(void (*f)(void* arg), void* a, Priority pri,
                void* tag = nullptr) override {
    return target_->Schedule(f, a, pri, tag);
  }

  int UnSchedule(void* tag, Priority pri) override {
    return target_->UnSchedule(tag, pri);
  }

  void StartThread(void (*f)(void*), void* a) override {
    return target_->StartThread(f, a);
  }
  void WaitForJoin() override { return target_->WaitForJoin(); }
  virtual unsigned int GetThreadPoolQueueLen(
      Priority pri = LOW) const override {
    return target_->GetThreadPoolQueueLen(pri);
  }
  virtual Status GetTestDirectory(std::string* path) override {
    return target_->GetTestDirectory(path);
  }
  virtual Status NewLogger(const std::string& fname,
                           shared_ptr<Logger>* result) override {
    return target_->NewLogger(fname, result);
  }
  uint64_t NowMicros() override { return target_->NowMicros(); }
  void SleepForMicroseconds(int micros) override {
    target_->SleepForMicroseconds(micros);
  }
  Status GetHostName(char* name, uint64_t len) override {
    return target_->GetHostName(name, len);
  }
  Status GetCurrentTime(int64_t* unix_time) override {
    return target_->GetCurrentTime(unix_time);
  }
  Status GetAbsolutePath(const std::string& db_path,
                         std::string* output_path) override {
    return target_->GetAbsolutePath(db_path, output_path);
  }
  void SetBackgroundThreads(int num, Priority pri) override {
    return target_->SetBackgroundThreads(num, pri);
  }

  void IncBackgroundThreadsIfNeeded(int num, Priority pri) override {
    return target_->IncBackgroundThreadsIfNeeded(num, pri);
  }

  void LowerThreadPoolIOPriority(Priority pool = LOW) override {
    target_->LowerThreadPoolIOPriority(pool);
  }

  std::string TimeToString(uint64_t time) override {
    return target_->TimeToString(time);
  }

  Status GetThreadList(std::vector<ThreadStatus>* thread_list) override {
    return target_->GetThreadList(thread_list);
  }

  ThreadStatusUpdater* GetThreadStatusUpdater() const override {
    return target_->GetThreadStatusUpdater();
  }

  uint64_t GetThreadID() const override {
    return target_->GetThreadID();
  }

 private:
  Env* target_;
};

// An implementation of WritableFile that forwards all calls to another
// WritableFile. May be useful to clients who wish to override just part of the
// functionality of another WritableFile.
// It's declared as friend of WritableFile to allow forwarding calls to
// protected virtual methods.
class WritableFileWrapper : public WritableFile {
 public:
  explicit WritableFileWrapper(WritableFile* t) : target_(t) { }

  Status Append(const Slice& data) override { return target_->Append(data); }
  Status PositionedAppend(const Slice& data, uint64_t offset) override {
    return target_->PositionedAppend(data, offset);
  }
  Status Truncate(uint64_t size) override { return target_->Truncate(size); }
  Status Close() override { return target_->Close(); }
  Status Flush() override { return target_->Flush(); }
  Status Sync() override { return target_->Sync(); }
  Status Fsync() override { return target_->Fsync(); }
  bool IsSyncThreadSafe() const override { return target_->IsSyncThreadSafe(); }
  void SetIOPriority(Env::IOPriority pri) override {
    target_->SetIOPriority(pri);
  }
  Env::IOPriority GetIOPriority() override { return target_->GetIOPriority(); }
  uint64_t GetFileSize() override { return target_->GetFileSize(); }
  void GetPreallocationStatus(size_t* block_size,
                              size_t* last_allocated_block) override {
    target_->GetPreallocationStatus(block_size, last_allocated_block);
  }
  size_t GetUniqueId(char* id, size_t max_size) const override {
    return target_->GetUniqueId(id, max_size);
  }
  Status InvalidateCache(size_t offset, size_t length) override {
    return target_->InvalidateCache(offset, length);
  }

 protected:
  Status Allocate(off_t offset, off_t len) override {
    return target_->Allocate(offset, len);
  }
  Status RangeSync(off_t offset, off_t nbytes) override {
    return target_->RangeSync(offset, nbytes);
  }

 private:
  WritableFile* target_;
};

// Returns a new environment that stores its data in memory and delegates
// all non-file-storage tasks to base_env. The caller must delete the result
// when it is no longer needed.
// *base_env must remain live while the result is in use.
Env* NewMemEnv(Env* base_env);

}  // namespace rocksdb

#endif  // STORAGE_ROCKSDB_INCLUDE_ENV_H_
#line 1 "/home/evan/source/rocksdb/include/rocksdb/options.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_ROCKSDB_INCLUDE_OPTIONS_H_
#define STORAGE_ROCKSDB_INCLUDE_OPTIONS_H_

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <memory>
#include <vector>
#include <limits>
#include <unordered_map>

#line 1 "/home/evan/source/rocksdb/include/rocksdb/version.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#define ROCKSDB_MAJOR 4
#define ROCKSDB_MINOR 1
#define ROCKSDB_PATCH 0

// Do not use these. We made the mistake of declaring macros starting with
// double underscore. Now we have to live with our choice. We'll deprecate these
// at some point
#define __ROCKSDB_MAJOR__ ROCKSDB_MAJOR
#define __ROCKSDB_MINOR__ ROCKSDB_MINOR
#define __ROCKSDB_PATCH__ ROCKSDB_PATCH
#line 20 "/home/evan/source/rocksdb/include/rocksdb/options.h"
#line 1 "/home/evan/source/rocksdb/include/rocksdb/listener.h"
// Copyright (c) 2014 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.


#include <string>
#include <vector>
#line 1 "/home/evan/source/rocksdb/include/rocksdb/compaction_job_stats.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <stddef.h>
#include <stdint.h>
#include <string>

namespace rocksdb {
struct CompactionJobStats {
  CompactionJobStats() { Reset(); }
  void Reset();
  // Aggregate the CompactionJobStats from another instance with this one
  void Add(const CompactionJobStats& stats);

  // the elapsed time in micro of this compaction.
  uint64_t elapsed_micros;

  // the number of compaction input records.
  uint64_t num_input_records;
  // the number of compaction input files.
  size_t num_input_files;
  // the number of compaction input files at the output level.
  size_t num_input_files_at_output_level;

  // the number of compaction output records.
  uint64_t num_output_records;
  // the number of compaction output files.
  size_t num_output_files;

  // true if the compaction is a manual compaction
  bool is_manual_compaction;

  // the size of the compaction input in bytes.
  uint64_t total_input_bytes;
  // the size of the compaction output in bytes.
  uint64_t total_output_bytes;

  // number of records being replaced by newer record associated with same key.
  // this could be a new value or a deletion entry for that key so this field
  // sums up all updated and deleted keys
  uint64_t num_records_replaced;

  // the sum of the uncompressed input keys in bytes.
  uint64_t total_input_raw_key_bytes;
  // the sum of the uncompressed input values in bytes.
  uint64_t total_input_raw_value_bytes;

  // the number of deletion entries before compaction. Deletion entries
  // can disappear after compaction because they expired
  uint64_t num_input_deletion_records;
  // number of deletion records that were found obsolete and discarded
  // because it is not possible to delete any more keys with this entry
  // (i.e. all possible deletions resulting from it have been completed)
  uint64_t num_expired_deletion_records;

  // number of corrupt keys (ParseInternalKey returned false when applied to
  // the key) encountered and written out.
  uint64_t num_corrupt_keys;

  // Following counters are only populated if
  // options.compaction_measure_io_stats = true;

  // Time spent on file's Append() call.
  uint64_t file_write_nanos;

  // Time spent on sync file range.
  uint64_t file_range_sync_nanos;

  // Time spent on file fsync.
  uint64_t file_fsync_nanos;

  // Time spent on preparing file write (falocate, etc)
  uint64_t file_prepare_write_nanos;

  // 0-terminated strings storing the first 8 bytes of the smallest and
  // largest key in the output.
  static const size_t kMaxPrefixLength = 8;

  std::string smallest_output_key_prefix;
  std::string largest_output_key_prefix;
};
}  // namespace rocksdb
#line 9 "/home/evan/source/rocksdb/include/rocksdb/listener.h"

namespace rocksdb {

class DB;
class Status;
struct CompactionJobStats;

struct TableFileCreationInfo {
  TableFileCreationInfo() = default;
  explicit TableFileCreationInfo(TableProperties&& prop) :
      table_properties(prop) {}
  // the name of the database where the file was created
  std::string db_name;
  // the name of the column family where the file was created.
  std::string cf_name;
  // the path to the created file.
  std::string file_path;
  // the size of the file.
  uint64_t file_size;
  // the id of the job (which could be flush or compaction) that
  // created the file.
  int job_id;
  // Detailed properties of the created file.
  TableProperties table_properties;
};


#ifndef ROCKSDB_LITE

struct TableFileDeletionInfo {
  // The name of the database where the file was deleted.
  std::string db_name;
  // The path to the deleted file.
  std::string file_path;
  // The id of the job which deleted the file.
  int job_id;
  // The status indicating whether the deletion was successfull or not.
  Status status;
};

struct FlushJobInfo {
  // the name of the column family
  std::string cf_name;
  // the path to the newly created file
  std::string file_path;
  // the id of the thread that completed this flush job.
  uint64_t thread_id;
  // the job id, which is unique in the same thread.
  int job_id;
  // If true, then rocksdb is currently slowing-down all writes to prevent
  // creating too many Level 0 files as compaction seems not able to
  // catch up the write request speed.  This indicates that there are
  // too many files in Level 0.
  bool triggered_writes_slowdown;
  // If true, then rocksdb is currently blocking any writes to prevent
  // creating more L0 files.  This indicates that there are too many
  // files in level 0.  Compactions should try to compact L0 files down
  // to lower levels as soon as possible.
  bool triggered_writes_stop;
  // The smallest sequence number in the newly created file
  SequenceNumber smallest_seqno;
  // The largest sequence number in the newly created file
  SequenceNumber largest_seqno;
};

struct CompactionJobInfo {
  CompactionJobInfo() = default;
  explicit CompactionJobInfo(const CompactionJobStats& _stats) :
      stats(_stats) {}

  // the name of the column family where the compaction happened.
  std::string cf_name;
  // the status indicating whether the compaction was successful or not.
  Status status;
  // the id of the thread that completed this compaction job.
  uint64_t thread_id;
  // the job id, which is unique in the same thread.
  int job_id;
  // the smallest input level of the compaction.
  int base_input_level;
  // the output level of the compaction.
  int output_level;
  // the names of the compaction input files.
  std::vector<std::string> input_files;
  // the names of the compaction output files.
  std::vector<std::string> output_files;
  // If non-null, this variable stores detailed information
  // about this compaction.
  CompactionJobStats stats;
};

// EventListener class contains a set of call-back functions that will
// be called when specific RocksDB event happens such as flush.  It can
// be used as a building block for developing custom features such as
// stats-collector or external compaction algorithm.
//
// Note that call-back functions should not run for an extended period of
// time before the function returns, otherwise RocksDB may be blocked.
// For example, it is not suggested to do DB::CompactFiles() (as it may
// run for a long while) or issue many of DB::Put() (as Put may be blocked
// in certain cases) in the same thread in the EventListener callback.
// However, doing DB::CompactFiles() and DB::Put() in another thread is
// considered safe.
//
// [Threading] All EventListener callback will be called using the
// actual thread that involves in that specific event.   For example, it
// is the RocksDB background flush thread that does the actual flush to
// call EventListener::OnFlushCompleted().
//
// [Locking] All EventListener callbacks are designed to be called without
// the current thread holding any DB mutex. This is to prevent potential
// deadlock and performance issue when using EventListener callback
// in a complex way. However, all EventListener call-back functions
// should not run for an extended period of time before the function
// returns, otherwise RocksDB may be blocked. For example, it is not
// suggested to do DB::CompactFiles() (as it may run for a long while)
// or issue many of DB::Put() (as Put may be blocked in certain cases)
// in the same thread in the EventListener callback. However, doing
// DB::CompactFiles() and DB::Put() in a thread other than the
// EventListener callback thread is considered safe.
class EventListener {
 public:
  // A call-back function to RocksDB which will be called whenever a
  // registered RocksDB flushes a file.  The default implementation is
  // no-op.
  //
  // Note that the this function must be implemented in a way such that
  // it should not run for an extended period of time before the function
  // returns.  Otherwise, RocksDB may be blocked.
  virtual void OnFlushCompleted(
      DB* db, const FlushJobInfo& flush_job_info) {}

  // A call-back function for RocksDB which will be called whenever
  // a SST file is deleted.  Different from OnCompactionCompleted and
  // OnFlushCompleted, this call-back is designed for external logging
  // service and thus only provide string parameters instead
  // of a pointer to DB.  Applications that build logic basic based
  // on file creations and deletions is suggested to implement
  // OnFlushCompleted and OnCompactionCompleted.
  //
  // Note that if applications would like to use the passed reference
  // outside this function call, they should make copies from the
  // returned value.
  virtual void OnTableFileDeleted(
      const TableFileDeletionInfo& info) {}

  // A call-back function for RocksDB which will be called whenever
  // a registered RocksDB compacts a file. The default implementation
  // is a no-op.
  //
  // Note that this function must be implemented in a way such that
  // it should not run for an extended period of time before the function
  // returns. Otherwise, RocksDB may be blocked.
  //
  // @param db a pointer to the rocksdb instance which just compacted
  //   a file.
  // @param ci a reference to a CompactionJobInfo struct. 'ci' is released
  //  after this function is returned, and must be copied if it is needed
  //  outside of this function.
  virtual void OnCompactionCompleted(DB *db, const CompactionJobInfo& ci) {}

  // A call-back function for RocksDB which will be called whenever
  // a SST file is created.  Different from OnCompactionCompleted and
  // OnFlushCompleted, this call-back is designed for external logging
  // service and thus only provide string parameters instead
  // of a pointer to DB.  Applications that build logic basic based
  // on file creations and deletions is suggested to implement
  // OnFlushCompleted and OnCompactionCompleted.
  //
  // Note that if applications would like to use the passed reference
  // outside this function call, they should make copies from these
  // returned value.
  virtual void OnTableFileCreated(
      const TableFileCreationInfo& info) {}

  virtual ~EventListener() {}
};

#else

class EventListener {
};

#endif  // ROCKSDB_LITE

}  // namespace rocksdb
#line 21 "/home/evan/source/rocksdb/include/rocksdb/options.h"
#line 1 "/home/evan/source/rocksdb/include/rocksdb/universal_compaction.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#ifndef STORAGE_ROCKSDB_UNIVERSAL_COMPACTION_OPTIONS_H
#define STORAGE_ROCKSDB_UNIVERSAL_COMPACTION_OPTIONS_H

#include <stdint.h>
#include <climits>
#include <vector>

namespace rocksdb {

//
// Algorithm used to make a compaction request stop picking new files
// into a single compaction run
//
enum CompactionStopStyle {
  kCompactionStopStyleSimilarSize, // pick files of similar size
  kCompactionStopStyleTotalSize    // total size of picked files > next file
};

class CompactionOptionsUniversal {
 public:

  // Percentage flexibilty while comparing file size. If the candidate file(s)
  // size is 1% smaller than the next file's size, then include next file into
  // this candidate set. // Default: 1
  unsigned int size_ratio;

  // The minimum number of files in a single compaction run. Default: 2
  unsigned int min_merge_width;

  // The maximum number of files in a single compaction run. Default: UINT_MAX
  unsigned int max_merge_width;

  // The size amplification is defined as the amount (in percentage) of
  // additional storage needed to store a single byte of data in the database.
  // For example, a size amplification of 2% means that a database that
  // contains 100 bytes of user-data may occupy upto 102 bytes of
  // physical storage. By this definition, a fully compacted database has
  // a size amplification of 0%. Rocksdb uses the following heuristic
  // to calculate size amplification: it assumes that all files excluding
  // the earliest file contribute to the size amplification.
  // Default: 200, which means that a 100 byte database could require upto
  // 300 bytes of storage.
  unsigned int max_size_amplification_percent;

  // If this option is set to be -1 (the default value), all the output files
  // will follow compression type specified.
  //
  // If this option is not negative, we will try to make sure compressed
  // size is just above this value. In normal cases, at least this percentage
  // of data will be compressed.
  // When we are compacting to a new file, here is the criteria whether
  // it needs to be compressed: assuming here are the list of files sorted
  // by generation time:
  //    A1...An B1...Bm C1...Ct
  // where A1 is the newest and Ct is the oldest, and we are going to compact
  // B1...Bm, we calculate the total size of all the files as total_size, as
  // well as  the total size of C1...Ct as total_C, the compaction output file
  // will be compressed iff
  //   total_C / total_size < this percentage
  // Default: -1
  int compression_size_percent;

  // The algorithm used to stop picking files into a single compaction run
  // Default: kCompactionStopStyleTotalSize
  CompactionStopStyle stop_style;

  // Option to optimize the universal multi level compaction by enabling
  // trivial move for non overlapping files.
  // Default: false
  bool allow_trivial_move;

  // Default set of parameters
  CompactionOptionsUniversal()
      : size_ratio(1),
        min_merge_width(2),
        max_merge_width(UINT_MAX),
        max_size_amplification_percent(200),
        compression_size_percent(-1),
        stop_style(kCompactionStopStyleTotalSize),
        allow_trivial_move(false) {}
};

}  // namespace rocksdb

#endif  // STORAGE_ROCKSDB_UNIVERSAL_COMPACTION_OPTIONS_H
#line 22 "/home/evan/source/rocksdb/include/rocksdb/options.h"

#ifdef max
#undef max
#endif

namespace rocksdb {

class Cache;
class CompactionFilter;
class CompactionFilterFactory;
class Comparator;
class Env;
enum InfoLogLevel : unsigned char;
class FilterPolicy;
class Logger;
class MergeOperator;
class Snapshot;
class TableFactory;
class MemTableRepFactory;
class TablePropertiesCollectorFactory;
class RateLimiter;
class DeleteScheduler;
class Slice;
class SliceTransform;
class Statistics;
class InternalKeyComparator;

// DB contents are stored in a set of blocks, each of which holds a
// sequence of key,value pairs.  Each block may be compressed before
// being stored in a file.  The following enum describes which
// compression method (if any) is used to compress a block.
enum CompressionType : char {
  // NOTE: do not change the values of existing entries, as these are
  // part of the persistent format on disk.
  kNoCompression = 0x0,
  kSnappyCompression = 0x1,
  kZlibCompression = 0x2,
  kBZip2Compression = 0x3,
  kLZ4Compression = 0x4,
  kLZ4HCCompression = 0x5,
  // zstd format is not finalized yet so it's subject to changes.
  kZSTDNotFinalCompression = 0x40,
};

enum CompactionStyle : char {
  // level based compaction style
  kCompactionStyleLevel = 0x0,
  // Universal compaction style
  // Not supported in ROCKSDB_LITE.
  kCompactionStyleUniversal = 0x1,
  // FIFO compaction style
  // Not supported in ROCKSDB_LITE
  kCompactionStyleFIFO = 0x2,
  // Disable background compaction. Compaction jobs are submitted
  // via CompactFiles().
  // Not supported in ROCKSDB_LITE
  kCompactionStyleNone = 0x3,
};

enum CompactionPri : char {
  // Slightly Priotize larger files by size compensated by #deletes
  kCompactionPriByCompensatedSize = 0x0,
  // First compact files whose data is oldest.
  kCompactionPriByLargestSeq = 0x1,
};

enum class WALRecoveryMode : char {
  // Original levelDB recovery
  // We tolerate incomplete record in trailing data on all logs
  // Use case : This is legacy behavior (default)
  kTolerateCorruptedTailRecords = 0x00,
  // Recover from clean shutdown
  // We don't expect to find any corruption in the WAL
  // Use case : This is ideal for unit tests and rare applications that
  // can require high consistency guarantee
  kAbsoluteConsistency = 0x01,
  // Recover to point-in-time consistency
  // We stop the WAL playback on discovering WAL inconsistency
  // Use case : Ideal for systems that have disk controller cache like
  // hard disk, SSD without super capacitor that store related data
  kPointInTimeRecovery = 0x02,
  // Recovery after a disaster
  // We ignore any corruption in the WAL and try to salvage as much data as
  // possible
  // Use case : Ideal for last ditch effort to recover data or systems that
  // operate with low grade unrelated data
  kSkipAnyCorruptedRecords = 0x03,
};

struct CompactionOptionsFIFO {
  // once the total sum of table files reaches this, we will delete the oldest
  // table file
  // Default: 1GB
  uint64_t max_table_files_size;

  CompactionOptionsFIFO() : max_table_files_size(1 * 1024 * 1024 * 1024) {}
};

// Compression options for different compression algorithms like Zlib
struct CompressionOptions {
  int window_bits;
  int level;
  int strategy;
  CompressionOptions() : window_bits(-14), level(-1), strategy(0) {}
  CompressionOptions(int wbits, int _lev, int _strategy)
      : window_bits(wbits), level(_lev), strategy(_strategy) {}
};

enum UpdateStatus {    // Return status For inplace update callback
  UPDATE_FAILED   = 0, // Nothing to update
  UPDATED_INPLACE = 1, // Value updated inplace
  UPDATED         = 2, // No inplace update. Merged value set
};

struct DbPath {
  std::string path;
  uint64_t target_size;  // Target size of total files under the path, in byte.

  DbPath() : target_size(0) {}
  DbPath(const std::string& p, uint64_t t) : path(p), target_size(t) {}
};

struct Options;

struct ColumnFamilyOptions {
  // Some functions that make it easier to optimize RocksDB

  // Use this if you don't need to keep the data sorted, i.e. you'll never use
  // an iterator, only Put() and Get() API calls
  //
  // Not supported in ROCKSDB_LITE
  ColumnFamilyOptions* OptimizeForPointLookup(
      uint64_t block_cache_size_mb);

  // Default values for some parameters in ColumnFamilyOptions are not
  // optimized for heavy workloads and big datasets, which means you might
  // observe write stalls under some conditions. As a starting point for tuning
  // RocksDB options, use the following two functions:
  // * OptimizeLevelStyleCompaction -- optimizes level style compaction
  // * OptimizeUniversalStyleCompaction -- optimizes universal style compaction
  // Universal style compaction is focused on reducing Write Amplification
  // Factor for big data sets, but increases Space Amplification. You can learn
  // more about the different styles here:
  // https://github.com/facebook/rocksdb/wiki/Rocksdb-Architecture-Guide
  // Make sure to also call IncreaseParallelism(), which will provide the
  // biggest performance gains.
  // Note: we might use more memory than memtable_memory_budget during high
  // write rate period
  //
  // OptimizeUniversalStyleCompaction is not supported in ROCKSDB_LITE
  ColumnFamilyOptions* OptimizeLevelStyleCompaction(
      uint64_t memtable_memory_budget = 512 * 1024 * 1024);
  ColumnFamilyOptions* OptimizeUniversalStyleCompaction(
      uint64_t memtable_memory_budget = 512 * 1024 * 1024);

  // -------------------
  // Parameters that affect behavior

  // Comparator used to define the order of keys in the table.
  // Default: a comparator that uses lexicographic byte-wise ordering
  //
  // REQUIRES: The client must ensure that the comparator supplied
  // here has the same name and orders keys *exactly* the same as the
  // comparator provided to previous open calls on the same DB.
  const Comparator* comparator;

  // REQUIRES: The client must provide a merge operator if Merge operation
  // needs to be accessed. Calling Merge on a DB without a merge operator
  // would result in Status::NotSupported. The client must ensure that the
  // merge operator supplied here has the same name and *exactly* the same
  // semantics as the merge operator provided to previous open calls on
  // the same DB. The only exception is reserved for upgrade, where a DB
  // previously without a merge operator is introduced to Merge operation
  // for the first time. It's necessary to specify a merge operator when
  // openning the DB in this case.
  // Default: nullptr
  std::shared_ptr<MergeOperator> merge_operator;

  // A single CompactionFilter instance to call into during compaction.
  // Allows an application to modify/delete a key-value during background
  // compaction.
  //
  // If the client requires a new compaction filter to be used for different
  // compaction runs, it can specify compaction_filter_factory instead of this
  // option.  The client should specify only one of the two.
  // compaction_filter takes precedence over compaction_filter_factory if
  // client specifies both.
  //
  // If multithreaded compaction is being used, the supplied CompactionFilter
  // instance may be used from different threads concurrently and so should be
  // thread-safe.
  //
  // Default: nullptr
  const CompactionFilter* compaction_filter;

  // This is a factory that provides compaction filter objects which allow
  // an application to modify/delete a key-value during background compaction.
  //
  // A new filter will be created on each compaction run.  If multithreaded
  // compaction is being used, each created CompactionFilter will only be used
  // from a single thread and so does not need to be thread-safe.
  //
  // Default: nullptr
  std::shared_ptr<CompactionFilterFactory> compaction_filter_factory;

  // -------------------
  // Parameters that affect performance

  // Amount of data to build up in memory (backed by an unsorted log
  // on disk) before converting to a sorted on-disk file.
  //
  // Larger values increase performance, especially during bulk loads.
  // Up to max_write_buffer_number write buffers may be held in memory
  // at the same time,
  // so you may wish to adjust this parameter to control memory usage.
  // Also, a larger write buffer will result in a longer recovery time
  // the next time the database is opened.
  //
  // Note that write_buffer_size is enforced per column family.
  // See db_write_buffer_size for sharing memory across column families.
  //
  // Default: 4MB
  //
  // Dynamically changeable through SetOptions() API
  size_t write_buffer_size;

  // The maximum number of write buffers that are built up in memory.
  // The default and the minimum number is 2, so that when 1 write buffer
  // is being flushed to storage, new writes can continue to the other
  // write buffer.
  //
  // Default: 2
  //
  // Dynamically changeable through SetOptions() API
  int max_write_buffer_number;

  // The minimum number of write buffers that will be merged together
  // before writing to storage.  If set to 1, then
  // all write buffers are fushed to L0 as individual files and this increases
  // read amplification because a get request has to check in all of these
  // files. Also, an in-memory merge may result in writing lesser
  // data to storage if there are duplicate records in each of these
  // individual write buffers.  Default: 1
  int min_write_buffer_number_to_merge;

  // The total maximum number of write buffers to maintain in memory including
  // copies of buffers that have already been flushed.  Unlike
  // max_write_buffer_number, this parameter does not affect flushing.
  // This controls the minimum amount of write history that will be available
  // in memory for conflict checking when Transactions are used.
  // If this value is too low, some transactions may fail at commit time due
  // to not being able to determine whether there were any write conflicts.
  //
  // Setting this value to 0 will cause write buffers to be freed immediately
  // after they are flushed.
  // If this value is set to -1, 'max_write_buffer_number' will be used.
  //
  // Default:
  // If using a TransactionDB/OptimisticTransactionDB, the default value will
  // be set to the value of 'max_write_buffer_number' if it is not explicitly
  // set by the user.  Otherwise, the default is 0.
  int max_write_buffer_number_to_maintain;

  // Compress blocks using the specified compression algorithm.  This
  // parameter can be changed dynamically.
  //
  // Default: kSnappyCompression, if it's supported. If snappy is not linked
  // with the library, the default is kNoCompression.
  //
  // Typical speeds of kSnappyCompression on an Intel(R) Core(TM)2 2.4GHz:
  //    ~200-500MB/s compression
  //    ~400-800MB/s decompression
  // Note that these speeds are significantly faster than most
  // persistent storage speeds, and therefore it is typically never
  // worth switching to kNoCompression.  Even if the input data is
  // incompressible, the kSnappyCompression implementation will
  // efficiently detect that and will switch to uncompressed mode.
  CompressionType compression;

  // Different levels can have different compression policies. There
  // are cases where most lower levels would like to use quick compression
  // algorithms while the higher levels (which have more data) use
  // compression algorithms that have better compression but could
  // be slower. This array, if non-empty, should have an entry for
  // each level of the database; these override the value specified in
  // the previous field 'compression'.
  //
  // NOTICE if level_compaction_dynamic_level_bytes=true,
  // compression_per_level[0] still determines L0, but other elements
  // of the array are based on base level (the level L0 files are merged
  // to), and may not match the level users see from info log for metadata.
  // If L0 files are merged to level-n, then, for i>0, compression_per_level[i]
  // determines compaction type for level n+i-1.
  // For example, if we have three 5 levels, and we determine to merge L0
  // data to L4 (which means L1..L3 will be empty), then the new files go to
  // L4 uses compression type compression_per_level[1].
  // If now L0 is merged to L2. Data goes to L2 will be compressed
  // according to compression_per_level[1], L3 using compression_per_level[2]
  // and L4 using compression_per_level[3]. Compaction for each level can
  // change when data grows.
  std::vector<CompressionType> compression_per_level;

  // different options for compression algorithms
  CompressionOptions compression_opts;

  // If non-nullptr, use the specified function to determine the
  // prefixes for keys.  These prefixes will be placed in the filter.
  // Depending on the workload, this can reduce the number of read-IOP
  // cost for scans when a prefix is passed via ReadOptions to
  // db.NewIterator().  For prefix filtering to work properly,
  // "prefix_extractor" and "comparator" must be such that the following
  // properties hold:
  //
  // 1) key.starts_with(prefix(key))
  // 2) Compare(prefix(key), key) <= 0.
  // 3) If Compare(k1, k2) <= 0, then Compare(prefix(k1), prefix(k2)) <= 0
  // 4) prefix(prefix(key)) == prefix(key)
  //
  // Default: nullptr
  std::shared_ptr<const SliceTransform> prefix_extractor;

  // Number of levels for this database
  int num_levels;

  // Number of files to trigger level-0 compaction. A value <0 means that
  // level-0 compaction will not be triggered by number of files at all.
  //
  // Default: 4
  //
  // Dynamically changeable through SetOptions() API
  int level0_file_num_compaction_trigger;

  // Soft limit on number of level-0 files. We start slowing down writes at this
  // point. A value <0 means that no writing slow down will be triggered by
  // number of files in level-0.
  //
  // Dynamically changeable through SetOptions() API
  int level0_slowdown_writes_trigger;

  // Maximum number of level-0 files.  We stop writes at this point.
  //
  // Dynamically changeable through SetOptions() API
  int level0_stop_writes_trigger;

  // This does not do anything anymore. Deprecated.
  int max_mem_compaction_level;

  // Target file size for compaction.
  // target_file_size_base is per-file size for level-1.
  // Target file size for level L can be calculated by
  // target_file_size_base * (target_file_size_multiplier ^ (L-1))
  // For example, if target_file_size_base is 2MB and
  // target_file_size_multiplier is 10, then each file on level-1 will
  // be 2MB, and each file on level 2 will be 20MB,
  // and each file on level-3 will be 200MB.
  //
  // Default: 2MB.
  //
  // Dynamically changeable through SetOptions() API
  uint64_t target_file_size_base;

  // By default target_file_size_multiplier is 1, which means
  // by default files in different levels will have similar size.
  //
  // Dynamically changeable through SetOptions() API
  int target_file_size_multiplier;

  // Control maximum total data size for a level.
  // max_bytes_for_level_base is the max total for level-1.
  // Maximum number of bytes for level L can be calculated as
  // (max_bytes_for_level_base) * (max_bytes_for_level_multiplier ^ (L-1))
  // For example, if max_bytes_for_level_base is 20MB, and if
  // max_bytes_for_level_multiplier is 10, total data size for level-1
  // will be 20MB, total file size for level-2 will be 200MB,
  // and total file size for level-3 will be 2GB.
  //
  // Default: 10MB.
  //
  // Dynamically changeable through SetOptions() API
  uint64_t max_bytes_for_level_base;

  // If true, RocksDB will pick target size of each level dynamically.
  // We will pick a base level b >= 1. L0 will be directly merged into level b,
  // instead of always into level 1. Level 1 to b-1 need to be empty.
  // We try to pick b and its target size so that
  // 1. target size is in the range of
  //   (max_bytes_for_level_base / max_bytes_for_level_multiplier,
  //    max_bytes_for_level_base]
  // 2. target size of the last level (level num_levels-1) equals to extra size
  //    of the level.
  // At the same time max_bytes_for_level_multiplier and
  // max_bytes_for_level_multiplier_additional are still satisfied.
  //
  // With this option on, from an empty DB, we make last level the base level,
  // which means merging L0 data into the last level, until it exceeds
  // max_bytes_for_level_base. And then we make the second last level to be
  // base level, to start to merge L0 data to second last level, with its
  // target size to be 1/max_bytes_for_level_multiplier of the last level's
  // extra size. After the data accumulates more so that we need to move the
  // base level to the third last one, and so on.
  //
  // For example, assume max_bytes_for_level_multiplier=10, num_levels=6,
  // and max_bytes_for_level_base=10MB.
  // Target sizes of level 1 to 5 starts with:
  // [- - - - 10MB]
  // with base level is level. Target sizes of level 1 to 4 are not applicable
  // because they will not be used.
  // Until the size of Level 5 grows to more than 10MB, say 11MB, we make
  // base target to level 4 and now the targets looks like:
  // [- - - 1.1MB 11MB]
  // While data are accumulated, size targets are tuned based on actual data
  // of level 5. When level 5 has 50MB of data, the target is like:
  // [- - - 5MB 50MB]
  // Until level 5's actual size is more than 100MB, say 101MB. Now if we keep
  // level 4 to be the base level, its target size needs to be 10.1MB, which
  // doesn't satisfy the target size range. So now we make level 3 the target
  // size and the target sizes of the levels look like:
  // [- - 1.01MB 10.1MB 101MB]
  // In the same way, while level 5 further grows, all levels' targets grow,
  // like
  // [- - 5MB 50MB 500MB]
  // Until level 5 exceeds 1000MB and becomes 1001MB, we make level 2 the
  // base level and make levels' target sizes like this:
  // [- 1.001MB 10.01MB 100.1MB 1001MB]
  // and go on...
  //
  // By doing it, we give max_bytes_for_level_multiplier a priority against
  // max_bytes_for_level_base, for a more predictable LSM tree shape. It is
  // useful to limit worse case space amplification.
  //
  // max_bytes_for_level_multiplier_additional is ignored with this flag on.
  //
  // Turning this feature on or off for an existing DB can cause unexpected
  // LSM tree structure so it's not recommended.
  //
  // NOTE: this option is experimental
  //
  // Default: false
  bool level_compaction_dynamic_level_bytes;

  // Default: 10.
  //
  // Dynamically changeable through SetOptions() API
  int max_bytes_for_level_multiplier;

  // Different max-size multipliers for different levels.
  // These are multiplied by max_bytes_for_level_multiplier to arrive
  // at the max-size of each level.
  //
  // Default: 1
  //
  // Dynamically changeable through SetOptions() API
  std::vector<int> max_bytes_for_level_multiplier_additional;

  // Maximum number of bytes in all compacted files.  We avoid expanding
  // the lower level file set of a compaction if it would make the
  // total compaction cover more than
  // (expanded_compaction_factor * targetFileSizeLevel()) many bytes.
  //
  // Dynamically changeable through SetOptions() API
  int expanded_compaction_factor;

  // Maximum number of bytes in all source files to be compacted in a
  // single compaction run. We avoid picking too many files in the
  // source level so that we do not exceed the total source bytes
  // for compaction to exceed
  // (source_compaction_factor * targetFileSizeLevel()) many bytes.
  // Default:1, i.e. pick maxfilesize amount of data as the source of
  // a compaction.
  //
  // Dynamically changeable through SetOptions() API
  int source_compaction_factor;

  // Control maximum bytes of overlaps in grandparent (i.e., level+2) before we
  // stop building a single file in a level->level+1 compaction.
  //
  // Dynamically changeable through SetOptions() API
  int max_grandparent_overlap_factor;

  // Puts are delayed to options.delayed_write_rate when any level has a
  // compaction score that exceeds soft_rate_limit. This is ignored when == 0.0.
  //
  // Default: 0 (disabled)
  //
  // Dynamically changeable through SetOptions() API
  double soft_rate_limit;

  // DEPRECATED -- this options is no longer usde
  double hard_rate_limit;

  // All writes are stopped if estimated bytes needed to be compaction exceed
  // this threshold.
  //
  // Default: 0 (disabled)
  uint64_t hard_pending_compaction_bytes_limit;

  // DEPRECATED -- this options is no longer used
  unsigned int rate_limit_delay_max_milliseconds;

  // size of one block in arena memory allocation.
  // If <= 0, a proper value is automatically calculated (usually 1/8 of
  // writer_buffer_size, rounded up to a multiple of 4KB).
  //
  // There are two additonal restriction of the The specified size:
  // (1) size should be in the range of [4096, 2 << 30] and
  // (2) be the multiple of the CPU word (which helps with the memory
  // alignment).
  //
  // We'll automatically check and adjust the size number to make sure it
  // conforms to the restrictions.
  //
  // Default: 0
  //
  // Dynamically changeable through SetOptions() API
  size_t arena_block_size;

  // Disable automatic compactions. Manual compactions can still
  // be issued on this column family
  //
  // Dynamically changeable through SetOptions() API
  bool disable_auto_compactions;

  // DEPREACTED
  // Does not have any effect.
  bool purge_redundant_kvs_while_flush;

  // The compaction style. Default: kCompactionStyleLevel
  CompactionStyle compaction_style;

  // If level compaction_style = kCompactionStyleLevel, for each level,
  // which files are prioritized to be picked to compact.
  // Default: kCompactionPriByCompensatedSize
  CompactionPri compaction_pri;

  // If true, compaction will verify checksum on every read that happens
  // as part of compaction
  //
  // Default: true
  //
  // Dynamically changeable through SetOptions() API
  bool verify_checksums_in_compaction;

  // The options needed to support Universal Style compactions
  CompactionOptionsUniversal compaction_options_universal;

  // The options for FIFO compaction style
  CompactionOptionsFIFO compaction_options_fifo;

  // Use KeyMayExist API to filter deletes when this is true.
  // If KeyMayExist returns false, i.e. the key definitely does not exist, then
  // the delete is a noop. KeyMayExist only incurs in-memory look up.
  // This optimization avoids writing the delete to storage when appropriate.
  //
  // Default: false
  //
  // Dynamically changeable through SetOptions() API
  bool filter_deletes;

  // An iteration->Next() sequentially skips over keys with the same
  // user-key unless this option is set. This number specifies the number
  // of keys (with the same userkey) that will be sequentially
  // skipped before a reseek is issued.
  //
  // Default: 8
  //
  // Dynamically changeable through SetOptions() API
  uint64_t max_sequential_skip_in_iterations;

  // This is a factory that provides MemTableRep objects.
  // Default: a factory that provides a skip-list-based implementation of
  // MemTableRep.
  std::shared_ptr<MemTableRepFactory> memtable_factory;

  // This is a factory that provides TableFactory objects.
  // Default: a block-based table factory that provides a default
  // implementation of TableBuilder and TableReader with default
  // BlockBasedTableOptions.
  std::shared_ptr<TableFactory> table_factory;

  // Block-based table related options are moved to BlockBasedTableOptions.
  // Related options that were originally here but now moved include:
  //   no_block_cache
  //   block_cache
  //   block_cache_compressed
  //   block_size
  //   block_size_deviation
  //   block_restart_interval
  //   filter_policy
  //   whole_key_filtering
  // If you'd like to customize some of these options, you will need to
  // use NewBlockBasedTableFactory() to construct a new table factory.

  // This option allows user to to collect their own interested statistics of
  // the tables.
  // Default: empty vector -- no user-defined statistics collection will be
  // performed.
  typedef std::vector<std::shared_ptr<TablePropertiesCollectorFactory>>
      TablePropertiesCollectorFactories;
  TablePropertiesCollectorFactories table_properties_collector_factories;

  // Allows thread-safe inplace updates. If this is true, there is no way to
  // achieve point-in-time consistency using snapshot or iterator (assuming
  // concurrent updates). Hence iterator and multi-get will return results
  // which are not consistent as of any point-in-time.
  // If inplace_callback function is not set,
  //   Put(key, new_value) will update inplace the existing_value iff
  //   * key exists in current memtable
  //   * new sizeof(new_value) <= sizeof(existing_value)
  //   * existing_value for that key is a put i.e. kTypeValue
  // If inplace_callback function is set, check doc for inplace_callback.
  // Default: false.
  bool inplace_update_support;

  // Number of locks used for inplace update
  // Default: 10000, if inplace_update_support = true, else 0.
  //
  // Dynamically changeable through SetOptions() API
  size_t inplace_update_num_locks;

  // existing_value - pointer to previous value (from both memtable and sst).
  //                  nullptr if key doesn't exist
  // existing_value_size - pointer to size of existing_value).
  //                       nullptr if key doesn't exist
  // delta_value - Delta value to be merged with the existing_value.
  //               Stored in transaction logs.
  // merged_value - Set when delta is applied on the previous value.

  // Applicable only when inplace_update_support is true,
  // this callback function is called at the time of updating the memtable
  // as part of a Put operation, lets say Put(key, delta_value). It allows the
  // 'delta_value' specified as part of the Put operation to be merged with
  // an 'existing_value' of the key in the database.

  // If the merged value is smaller in size that the 'existing_value',
  // then this function can update the 'existing_value' buffer inplace and
  // the corresponding 'existing_value'_size pointer, if it wishes to.
  // The callback should return UpdateStatus::UPDATED_INPLACE.
  // In this case. (In this case, the snapshot-semantics of the rocksdb
  // Iterator is not atomic anymore).

  // If the merged value is larger in size than the 'existing_value' or the
  // application does not wish to modify the 'existing_value' buffer inplace,
  // then the merged value should be returned via *merge_value. It is set by
  // merging the 'existing_value' and the Put 'delta_value'. The callback should
  // return UpdateStatus::UPDATED in this case. This merged value will be added
  // to the memtable.

  // If merging fails or the application does not wish to take any action,
  // then the callback should return UpdateStatus::UPDATE_FAILED.

  // Please remember that the original call from the application is Put(key,
  // delta_value). So the transaction log (if enabled) will still contain (key,
  // delta_value). The 'merged_value' is not stored in the transaction log.
  // Hence the inplace_callback function should be consistent across db reopens.

  // Default: nullptr
  UpdateStatus (*inplace_callback)(char* existing_value,
                                   uint32_t* existing_value_size,
                                   Slice delta_value,
                                   std::string* merged_value);

  // if prefix_extractor is set and bloom_bits is not 0, create prefix bloom
  // for memtable
  //
  // Dynamically changeable through SetOptions() API
  uint32_t memtable_prefix_bloom_bits;

  // number of hash probes per key
  //
  // Dynamically changeable through SetOptions() API
  uint32_t memtable_prefix_bloom_probes;

  // Page size for huge page TLB for bloom in memtable. If <=0, not allocate
  // from huge page TLB but from malloc.
  // Need to reserve huge pages for it to be allocated. For example:
  //      sysctl -w vm.nr_hugepages=20
  // See linux doc Documentation/vm/hugetlbpage.txt
  //
  // Dynamically changeable through SetOptions() API
  size_t memtable_prefix_bloom_huge_page_tlb_size;

  // Control locality of bloom filter probes to improve cache miss rate.
  // This option only applies to memtable prefix bloom and plaintable
  // prefix bloom. It essentially limits every bloom checking to one cache line.
  // This optimization is turned off when set to 0, and positive number to turn
  // it on.
  // Default: 0
  uint32_t bloom_locality;

  // Maximum number of successive merge operations on a key in the memtable.
  //
  // When a merge operation is added to the memtable and the maximum number of
  // successive merges is reached, the value of the key will be calculated and
  // inserted into the memtable instead of the merge operation. This will
  // ensure that there are never more than max_successive_merges merge
  // operations in the memtable.
  //
  // Default: 0 (disabled)
  //
  // Dynamically changeable through SetOptions() API
  size_t max_successive_merges;

  // The number of partial merge operands to accumulate before partial
  // merge will be performed. Partial merge will not be called
  // if the list of values to merge is less than min_partial_merge_operands.
  //
  // If min_partial_merge_operands < 2, then it will be treated as 2.
  //
  // Default: 2
  uint32_t min_partial_merge_operands;

  // This flag specifies that the implementation should optimize the filters
  // mainly for cases where keys are found rather than also optimize for keys
  // missed. This would be used in cases where the application knows that
  // there are very few misses or the performance in the case of misses is not
  // important.
  //
  // For now, this flag allows us to not store filters for the last level i.e
  // the largest level which contains data of the LSM store. For keys which
  // are hits, the filters in this level are not useful because we will search
  // for the data anyway. NOTE: the filters in other levels are still useful
  // even for key hit because they tell us whether to look in that level or go
  // to the higher level.
  //
  // Default: false
  bool optimize_filters_for_hits;

  // After writing every SST file, reopen it and read all the keys.
  // Default: false
  bool paranoid_file_checks;

  // Measure IO stats in compactions, if true.
  // Default: false
  bool compaction_measure_io_stats;

  // Create ColumnFamilyOptions with default values for all fields
  ColumnFamilyOptions();
  // Create ColumnFamilyOptions from Options
  explicit ColumnFamilyOptions(const Options& options);

  void Dump(Logger* log) const;
};

struct DBOptions {
  // Some functions that make it easier to optimize RocksDB

#ifndef ROCKSDB_LITE
  // By default, RocksDB uses only one background thread for flush and
  // compaction. Calling this function will set it up such that total of
  // `total_threads` is used. Good value for `total_threads` is the number of
  // cores. You almost definitely want to call this function if your system is
  // bottlenecked by RocksDB.
  DBOptions* IncreaseParallelism(int total_threads = 16);
#endif  // ROCKSDB_LITE

  // If true, the database will be created if it is missing.
  // Default: false
  bool create_if_missing;

  // If true, missing column families will be automatically created.
  // Default: false
  bool create_missing_column_families;

  // If true, an error is raised if the database already exists.
  // Default: false
  bool error_if_exists;

  // If true, RocksDB will aggressively check consistency of the data.
  // Also, if any of the  writes to the database fails (Put, Delete, Merge,
  // Write), the database will switch to read-only mode and fail all other
  // Write operations.
  // In most cases you want this to be set to true.
  // Default: true
  bool paranoid_checks;

  // Use the specified object to interact with the environment,
  // e.g. to read/write files, schedule background work, etc.
  // Default: Env::Default()
  Env* env;

  // Use to control write rate of flush and compaction. Flush has higher
  // priority than compaction. Rate limiting is disabled if nullptr.
  // If rate limiter is enabled, bytes_per_sync is set to 1MB by default.
  // Default: nullptr
  std::shared_ptr<RateLimiter> rate_limiter;

  // Use to control files deletion rate, can be used among multiple
  // RocksDB instances. delete_scheduler is only used to delete table files that
  // need to be deleted from the first db_path (db_name if db_paths is empty),
  // other files types and other db_paths wont be affected by delete_scheduler.
  // Default: nullptr (disabled)
  std::shared_ptr<DeleteScheduler> delete_scheduler;

  // Any internal progress/error information generated by the db will
  // be written to info_log if it is non-nullptr, or to a file stored
  // in the same directory as the DB contents if info_log is nullptr.
  // Default: nullptr
  std::shared_ptr<Logger> info_log;

  InfoLogLevel info_log_level;

  // Number of open files that can be used by the DB.  You may need to
  // increase this if your database has a large working set. Value -1 means
  // files opened are always kept open. You can estimate number of files based
  // on target_file_size_base and target_file_size_multiplier for level-based
  // compaction. For universal-style compaction, you can usually set it to -1.
  // Default: 5000 or ulimit value of max open files (whichever is smaller)
  int max_open_files;

  // If max_open_files is -1, DB will open all files on DB::Open(). You can
  // use this option to increase the number of threads used to open the files.
  // Default: 1
  int max_file_opening_threads;

  // Once write-ahead logs exceed this size, we will start forcing the flush of
  // column families whose memtables are backed by the oldest live WAL file
  // (i.e. the ones that are causing all the space amplification). If set to 0
  // (default), we will dynamically choose the WAL size limit to be
  // [sum of all write_buffer_size * max_write_buffer_number] * 4
  // Default: 0
  uint64_t max_total_wal_size;

  // If non-null, then we should collect metrics about database operations
  // Statistics objects should not be shared between DB instances as
  // it does not use any locks to prevent concurrent updates.
  std::shared_ptr<Statistics> statistics;

  // If true, then the contents of manifest and data files are not synced
  // to stable storage. Their contents remain in the OS buffers till the
  // OS decides to flush them. This option is good for bulk-loading
  // of data. Once the bulk-loading is complete, please issue a
  // sync to the OS to flush all dirty buffesrs to stable storage.
  // Default: false
  bool disableDataSync;

  // If true, then every store to stable storage will issue a fsync.
  // If false, then every store to stable storage will issue a fdatasync.
  // This parameter should be set to true while storing data to
  // filesystem like ext3 that can lose files after a reboot.
  // Default: false
  bool use_fsync;

  // A list of paths where SST files can be put into, with its target size.
  // Newer data is placed into paths specified earlier in the vector while
  // older data gradually moves to paths specified later in the vector.
  //
  // For example, you have a flash device with 10GB allocated for the DB,
  // as well as a hard drive of 2TB, you should config it to be:
  //   [{"/flash_path", 10GB}, {"/hard_drive", 2TB}]
  //
  // The system will try to guarantee data under each path is close to but
  // not larger than the target size. But current and future file sizes used
  // by determining where to place a file are based on best-effort estimation,
  // which means there is a chance that the actual size under the directory
  // is slightly more than target size under some workloads. User should give
  // some buffer room for those cases.
  //
  // If none of the paths has sufficient room to place a file, the file will
  // be placed to the last path anyway, despite to the target size.
  //
  // Placing newer data to ealier paths is also best-efforts. User should
  // expect user files to be placed in higher levels in some extreme cases.
  //
  // If left empty, only one path will be used, which is db_name passed when
  // opening the DB.
  // Default: empty
  std::vector<DbPath> db_paths;

  // This specifies the info LOG dir.
  // If it is empty, the log files will be in the same dir as data.
  // If it is non empty, the log files will be in the specified dir,
  // and the db data dir's absolute path will be used as the log file
  // name's prefix.
  std::string db_log_dir;

  // This specifies the absolute dir path for write-ahead logs (WAL).
  // If it is empty, the log files will be in the same dir as data,
  //   dbname is used as the data dir by default
  // If it is non empty, the log files will be in kept the specified dir.
  // When destroying the db,
  //   all log files in wal_dir and the dir itself is deleted
  std::string wal_dir;

  // The periodicity when obsolete files get deleted. The default
  // value is 6 hours. The files that get out of scope by compaction
  // process will still get automatically delete on every compaction,
  // regardless of this setting
  uint64_t delete_obsolete_files_period_micros;

  // Maximum number of concurrent background compaction jobs, submitted to
  // the default LOW priority thread pool.
  // If you're increasing this, also consider increasing number of threads in
  // LOW priority thread pool. For more information, see
  // Env::SetBackgroundThreads
  // Default: 1
  int max_background_compactions;

  // This value represents the maximum number of threads that will
  // concurrently perform a compaction job by breaking it into multiple,
  // smaller ones that are run simultaneously.
  // Default: 1 (i.e. no subcompactions)
  uint32_t max_subcompactions;

  // Maximum number of concurrent background memtable flush jobs, submitted to
  // the HIGH priority thread pool.
  //
  // By default, all background jobs (major compaction and memtable flush) go
  // to the LOW priority pool. If this option is set to a positive number,
  // memtable flush jobs will be submitted to the HIGH priority pool.
  // It is important when the same Env is shared by multiple db instances.
  // Without a separate pool, long running major compaction jobs could
  // potentially block memtable flush jobs of other db instances, leading to
  // unnecessary Put stalls.
  //
  // If you're increasing this, also consider increasing number of threads in
  // HIGH priority thread pool. For more information, see
  // Env::SetBackgroundThreads
  // Default: 1
  int max_background_flushes;

  // Specify the maximal size of the info log file. If the log file
  // is larger than `max_log_file_size`, a new info log file will
  // be created.
  // If max_log_file_size == 0, all logs will be written to one
  // log file.
  size_t max_log_file_size;

  // Time for the info log file to roll (in seconds).
  // If specified with non-zero value, log file will be rolled
  // if it has been active longer than `log_file_time_to_roll`.
  // Default: 0 (disabled)
  size_t log_file_time_to_roll;

  // Maximal info log files to be kept.
  // Default: 1000
  size_t keep_log_file_num;

  // manifest file is rolled over on reaching this limit.
  // The older manifest file be deleted.
  // The default value is MAX_INT so that roll-over does not take place.
  uint64_t max_manifest_file_size;

  // Number of shards used for table cache.
  int table_cache_numshardbits;

  // DEPRECATED
  // int table_cache_remove_scan_count_limit;

  // The following two fields affect how archived logs will be deleted.
  // 1. If both set to 0, logs will be deleted asap and will not get into
  //    the archive.
  // 2. If WAL_ttl_seconds is 0 and WAL_size_limit_MB is not 0,
  //    WAL files will be checked every 10 min and if total size is greater
  //    then WAL_size_limit_MB, they will be deleted starting with the
  //    earliest until size_limit is met. All empty files will be deleted.
  // 3. If WAL_ttl_seconds is not 0 and WAL_size_limit_MB is 0, then
  //    WAL files will be checked every WAL_ttl_secondsi / 2 and those that
  //    are older than WAL_ttl_seconds will be deleted.
  // 4. If both are not 0, WAL files will be checked every 10 min and both
  //    checks will be performed with ttl being first.
  uint64_t WAL_ttl_seconds;
  uint64_t WAL_size_limit_MB;

  // Number of bytes to preallocate (via fallocate) the manifest
  // files.  Default is 4mb, which is reasonable to reduce random IO
  // as well as prevent overallocation for mounts that preallocate
  // large amounts of data (such as xfs's allocsize option).
  size_t manifest_preallocation_size;

  // Data being read from file storage may be buffered in the OS
  // Default: true
  bool allow_os_buffer;

  // Allow the OS to mmap file for reading sst tables. Default: false
  bool allow_mmap_reads;

  // Allow the OS to mmap file for writing.
  // DB::SyncWAL() only works if this is set to false.
  // Default: false
  bool allow_mmap_writes;

  // If false, fallocate() calls are bypassed
  bool allow_fallocate;

  // Disable child process inherit open files. Default: true
  bool is_fd_close_on_exec;

  // DEPRECATED -- this options is no longer used
  bool skip_log_error_on_recovery;

  // if not zero, dump rocksdb.stats to LOG every stats_dump_period_sec
  // Default: 600 (10 min)
  unsigned int stats_dump_period_sec;

  // If set true, will hint the underlying file system that the file
  // access pattern is random, when a sst file is opened.
  // Default: true
  bool advise_random_on_open;

  // Amount of data to build up in memtables across all column
  // families before writing to disk.
  //
  // This is distinct from write_buffer_size, which enforces a limit
  // for a single memtable.
  //
  // This feature is disabled by default. Specify a non-zero value
  // to enable it.
  //
  // Default: 0 (disabled)
  size_t db_write_buffer_size;

  // Specify the file access pattern once a compaction is started.
  // It will be applied to all input files of a compaction.
  // Default: NORMAL
  enum AccessHint {
      NONE,
      NORMAL,
      SEQUENTIAL,
      WILLNEED
  };
  AccessHint access_hint_on_compaction_start;

  // If true, always create a new file descriptor and new table reader
  // for compaction inputs. Turn this parameter on may introduce extra
  // memory usage in the table reader, if it allocates extra memory
  // for indexes. This will allow file descriptor prefetch options
  // to be set for compaction input files and not to impact file
  // descriptors for the same file used by user queries.
  // Suggest to enable BlockBasedTableOptions.cache_index_and_filter_blocks
  // for this mode if using block-based table.
  //
  // Default: false
  bool new_table_reader_for_compaction_inputs;

  // If non-zero, we perform bigger reads when doing compaction. If you're
  // running RocksDB on spinning disks, you should set this to at least 2MB.
  // That way RocksDB's compaction is doing sequential instead of random reads.
  //
  // When non-zero, we also force new_table_reader_for_compaction_inputs to
  // true.
  //
  // Default: 0
  size_t compaction_readahead_size;

  // Use adaptive mutex, which spins in the user space before resorting
  // to kernel. This could reduce context switch when the mutex is not
  // heavily contended. However, if the mutex is hot, we could end up
  // wasting spin time.
  // Default: false
  bool use_adaptive_mutex;

  // Create DBOptions with default values for all fields
  DBOptions();
  // Create DBOptions from Options
  explicit DBOptions(const Options& options);

  void Dump(Logger* log) const;

  // Allows OS to incrementally sync files to disk while they are being
  // written, asynchronously, in the background. This operation can be used
  // to smooth out write I/Os over time. Users shouldn't reply on it for
  // persistency guarantee.
  // Issue one request for every bytes_per_sync written. 0 turns it off.
  // Default: 0
  //
  // You may consider using rate_limiter to regulate write rate to device.
  // When rate limiter is enabled, it automatically enables bytes_per_sync
  // to 1MB.
  //
  // This option applies to table files
  uint64_t bytes_per_sync;

  // Same as bytes_per_sync, but applies to WAL files
  // Default: 0, turned off
  uint64_t wal_bytes_per_sync;

  // A vector of EventListeners which call-back functions will be called
  // when specific RocksDB event happens.
  std::vector<std::shared_ptr<EventListener>> listeners;

  // If true, then the status of the threads involved in this DB will
  // be tracked and available via GetThreadList() API.
  //
  // Default: false
  bool enable_thread_tracking;

  // The limited write rate to DB if soft_rate_limit or
  // level0_slowdown_writes_trigger is triggered. It is calculated using
  // size of user write requests before compression.
  // Unit: byte per second.
  //
  // Default: 1MB/s
  uint64_t delayed_write_rate;

  // If true, then DB::Open() will not update the statistics used to optimize
  // compaction decision by loading table properties from many files.
  // Turning off this feature will improve DBOpen time especially in
  // disk environment.
  //
  // Default: false
  bool skip_stats_update_on_db_open;

  // Recovery mode to control the consistency while replaying WAL
  // Default: kTolerateCorruptedTailRecords
  WALRecoveryMode wal_recovery_mode;

  // A global cache for table-level rows.
  // Default: nullptr (disabled)
  // Not supported in ROCKSDB_LITE mode!
  std::shared_ptr<Cache> row_cache;
};

// Options to control the behavior of a database (passed to DB::Open)
struct Options : public DBOptions, public ColumnFamilyOptions {
  // Create an Options object with default values for all fields.
  Options() : DBOptions(), ColumnFamilyOptions() {}

  Options(const DBOptions& db_options,
          const ColumnFamilyOptions& column_family_options)
      : DBOptions(db_options), ColumnFamilyOptions(column_family_options) {}

  void Dump(Logger* log) const;

  void DumpCFOptions(Logger* log) const;

  // Set appropriate parameters for bulk loading.
  // The reason that this is a function that returns "this" instead of a
  // constructor is to enable chaining of multiple similar calls in the future.
  //

  // All data will be in level 0 without any automatic compaction.
  // It's recommended to manually call CompactRange(NULL, NULL) before reading
  // from the database, because otherwise the read can be very slow.
  Options* PrepareForBulkLoad();
};

//
// An application can issue a read request (via Get/Iterators) and specify
// if that read should process data that ALREADY resides on a specified cache
// level. For example, if an application specifies kBlockCacheTier then the
// Get call will process data that is already processed in the memtable or
// the block cache. It will not page in data from the OS cache or data that
// resides in storage.
enum ReadTier {
  kReadAllTier = 0x0,    // data in memtable, block cache, OS cache or storage
  kBlockCacheTier = 0x1  // data in memtable or block cache
};

// Options that control read operations
struct ReadOptions {
  // If true, all data read from underlying storage will be
  // verified against corresponding checksums.
  // Default: true
  bool verify_checksums;

  // Should the "data block"/"index block"/"filter block" read for this
  // iteration be cached in memory?
  // Callers may wish to set this field to false for bulk scans.
  // Default: true
  bool fill_cache;

  // If this option is set and memtable implementation allows, Seek
  // might only return keys with the same prefix as the seek-key
  //
  // ! DEPRECATED: prefix_seek is on by default when prefix_extractor
  // is configured
  // bool prefix_seek;

  // If "snapshot" is non-nullptr, read as of the supplied snapshot
  // (which must belong to the DB that is being read and which must
  // not have been released).  If "snapshot" is nullptr, use an impliicit
  // snapshot of the state at the beginning of this read operation.
  // Default: nullptr
  const Snapshot* snapshot;

  // If "prefix" is non-nullptr, and ReadOptions is being passed to
  // db.NewIterator, only return results when the key begins with this
  // prefix.  This field is ignored by other calls (e.g., Get).
  // Options.prefix_extractor must also be set, and
  // prefix_extractor.InRange(prefix) must be true.  The iterator
  // returned by NewIterator when this option is set will behave just
  // as if the underlying store did not contain any non-matching keys,
  // with two exceptions.  Seek() only accepts keys starting with the
  // prefix, and SeekToLast() is not supported.  prefix filter with this
  // option will sometimes reduce the number of read IOPs.
  // Default: nullptr
  //
  // ! DEPRECATED
  // const Slice* prefix;

  // "iterate_upper_bound" defines the extent upto which the forward iterator
  // can returns entries. Once the bound is reached, Valid() will be false.
  // "iterate_upper_bound" is exclusive ie the bound value is
  // not a valid entry.  If iterator_extractor is not null, the Seek target
  // and iterator_upper_bound need to have the same prefix.
  // This is because ordering is not guaranteed outside of prefix domain.
  // There is no lower bound on the iterator. If needed, that can be easily
  // implemented
  //
  // Default: nullptr
  const Slice* iterate_upper_bound;

  // Specify if this read request should process data that ALREADY
  // resides on a particular cache. If the required data is not
  // found at the specified cache, then Status::Incomplete is returned.
  // Default: kReadAllTier
  ReadTier read_tier;

  // Specify to create a tailing iterator -- a special iterator that has a
  // view of the complete database (i.e. it can also be used to read newly
  // added data) and is optimized for sequential reads. It will return records
  // that were inserted into the database after the creation of the iterator.
  // Default: false
  // Not supported in ROCKSDB_LITE mode!
  bool tailing;

  // Specify to create a managed iterator -- a special iterator that
  // uses less resources by having the ability to free its underlying
  // resources on request.
  // Default: false
  // Not supported in ROCKSDB_LITE mode!
  bool managed;

  // Enable a total order seek regardless of index format (e.g. hash index)
  // used in the table. Some table format (e.g. plain table) may not support
  // this option.
  bool total_order_seek;

  ReadOptions();
  ReadOptions(bool cksum, bool cache);
};

// Options that control write operations
struct WriteOptions {
  // If true, the write will be flushed from the operating system
  // buffer cache (by calling WritableFile::Sync()) before the write
  // is considered complete.  If this flag is true, writes will be
  // slower.
  //
  // If this flag is false, and the machine crashes, some recent
  // writes may be lost.  Note that if it is just the process that
  // crashes (i.e., the machine does not reboot), no writes will be
  // lost even if sync==false.
  //
  // In other words, a DB write with sync==false has similar
  // crash semantics as the "write()" system call.  A DB write
  // with sync==true has similar crash semantics to a "write()"
  // system call followed by "fdatasync()".
  //
  // Default: false
  bool sync;

  // If true, writes will not first go to the write ahead log,
  // and the write may got lost after a crash.
  bool disableWAL;

  // The option is deprecated. It's not used anymore.
  uint64_t timeout_hint_us;

  // If true and if user is trying to write to column families that don't exist
  // (they were dropped),  ignore the write (don't return an error). If there
  // are multiple writes in a WriteBatch, other writes will succeed.
  // Default: false
  bool ignore_missing_column_families;

  WriteOptions()
      : sync(false),
        disableWAL(false),
        timeout_hint_us(0),
        ignore_missing_column_families(false) {}
};

// Options that control flush operations
struct FlushOptions {
  // If true, the flush will wait until the flush is done.
  // Default: true
  bool wait;

  FlushOptions() : wait(true) {}
};

// Get options based on some guidelines. Now only tune parameter based on
// flush/compaction and fill default parameters for other parameters.
// total_write_buffer_limit: budget for memory spent for mem tables
// read_amplification_threshold: comfortable value of read amplification
// write_amplification_threshold: comfortable value of write amplification.
// target_db_size: estimated total DB size.
extern Options GetOptions(size_t total_write_buffer_limit,
                          int read_amplification_threshold = 8,
                          int write_amplification_threshold = 32,
                          uint64_t target_db_size = 68719476736 /* 64GB */);

// CompactionOptions are used in CompactFiles() call.
struct CompactionOptions {
  // Compaction output compression type
  // Default: snappy
  CompressionType compression;
  // Compaction will create files of size `output_file_size_limit`.
  // Default: MAX, which means that compaction will create a single file
  uint64_t output_file_size_limit;

  CompactionOptions()
      : compression(kSnappyCompression),
        output_file_size_limit(std::numeric_limits<uint64_t>::max()) {}
};

// For level based compaction, we can configure if we want to skip/force
// bottommost level compaction.
enum class BottommostLevelCompaction {
  // Skip bottommost level compaction
  kSkip,
  // Only compact bottommost level if there is a compaction filter
  // This is the default option
  kIfHaveCompactionFilter,
  // Always compact bottommost level
  kForce,
};

// CompactRangeOptions is used by CompactRange() call.
struct CompactRangeOptions {
  // If true, compacted files will be moved to the minimum level capable
  // of holding the data or given level (specified non-negative target_level).
  bool change_level = false;
  // If change_level is true and target_level have non-negative value, compacted
  // files will be moved to target_level.
  int target_level = -1;
  // Compaction outputs will be placed in options.db_paths[target_path_id].
  // Behavior is undefined if target_path_id is out of range.
  uint32_t target_path_id = 0;
  // By default level based compaction will only compact the bottommost level
  // if there is a compaction filter
  BottommostLevelCompaction bottommost_level_compaction =
      BottommostLevelCompaction::kIfHaveCompactionFilter;
};
}  // namespace rocksdb

#endif  // STORAGE_ROCKSDB_INCLUDE_OPTIONS_H_
#line 1 "/home/evan/source/rocksdb/include/rocksdb/immutable_options.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.


#include <string>
#include <vector>

namespace rocksdb {

// ImmutableCFOptions is a data struct used by RocksDB internal. It contains a
// subset of Options that should not be changed during the entire lifetime
// of DB. You shouldn't need to access this data structure unless you are
// implementing a new TableFactory. Raw pointers defined in this struct do
// not have ownership to the data they point to. Options contains shared_ptr
// to these data.
struct ImmutableCFOptions {
  explicit ImmutableCFOptions(const Options& options);

  CompactionStyle compaction_style;

  CompactionOptionsUniversal compaction_options_universal;
  CompactionOptionsFIFO compaction_options_fifo;

  const SliceTransform* prefix_extractor;

  const Comparator* comparator;

  MergeOperator* merge_operator;

  const CompactionFilter* compaction_filter;

  CompactionFilterFactory* compaction_filter_factory;

  bool inplace_update_support;

  UpdateStatus (*inplace_callback)(char* existing_value,
                                   uint32_t* existing_value_size,
                                   Slice delta_value,
                                   std::string* merged_value);

  Logger* info_log;

  Statistics* statistics;

  InfoLogLevel info_log_level;

  Env* env;

  // Allow the OS to mmap file for reading sst tables. Default: false
  bool allow_mmap_reads;

  // Allow the OS to mmap file for writing. Default: false
  bool allow_mmap_writes;

  std::vector<DbPath> db_paths;

  MemTableRepFactory* memtable_factory;

  TableFactory* table_factory;

  Options::TablePropertiesCollectorFactories
    table_properties_collector_factories;

  bool advise_random_on_open;

  // This options is required by PlainTableReader. May need to move it
  // to PlainTalbeOptions just like bloom_bits_per_key
  uint32_t bloom_locality;

  bool purge_redundant_kvs_while_flush;

  uint32_t min_partial_merge_operands;

  bool disable_data_sync;

  bool use_fsync;

  CompressionType compression;

  std::vector<CompressionType> compression_per_level;

  CompressionOptions compression_opts;

  bool level_compaction_dynamic_level_bytes;

  Options::AccessHint access_hint_on_compaction_start;

  bool new_table_reader_for_compaction_inputs;

  size_t compaction_readahead_size;

  int num_levels;

  bool optimize_filters_for_hits;

  // A vector of EventListeners which call-back functions will be called
  // when specific RocksDB event happens.
  std::vector<std::shared_ptr<EventListener>> listeners;

  std::shared_ptr<Cache> row_cache;
};

}  // namespace rocksdb
#line 1 "/home/evan/source/rocksdb/include/rocksdb/db.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_ROCKSDB_INCLUDE_DB_H_
#define STORAGE_ROCKSDB_INCLUDE_DB_H_

#include <stdint.h>
#include <stdio.h>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#line 1 "/home/evan/source/rocksdb/include/rocksdb/metadata.h"
// Copyright (c) 2014, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.


#include <stdint.h>

#include <limits>
#include <string>
#include <vector>


namespace rocksdb {
struct ColumnFamilyMetaData;
struct LevelMetaData;
struct SstFileMetaData;

// The metadata that describes a column family.
struct ColumnFamilyMetaData {
  ColumnFamilyMetaData() : size(0), name("") {}
  ColumnFamilyMetaData(const std::string& _name, uint64_t _size,
                       const std::vector<LevelMetaData>&& _levels) :
      size(_size), name(_name), levels(_levels) {}

  // The size of this column family in bytes, which is equal to the sum of
  // the file size of its "levels".
  uint64_t size;
  // The number of files in this column family.
  size_t file_count;
  // The name of the column family.
  std::string name;
  // The metadata of all levels in this column family.
  std::vector<LevelMetaData> levels;
};

// The metadata that describes a level.
struct LevelMetaData {
  LevelMetaData(int _level, uint64_t _size,
                const std::vector<SstFileMetaData>&& _files) :
      level(_level), size(_size),
      files(_files) {}

  // The level which this meta data describes.
  const int level;
  // The size of this level in bytes, which is equal to the sum of
  // the file size of its "files".
  const uint64_t size;
  // The metadata of all sst files in this level.
  const std::vector<SstFileMetaData> files;
};

// The metadata that describes a SST file.
struct SstFileMetaData {
  SstFileMetaData() {}
  SstFileMetaData(const std::string& _file_name,
                  const std::string& _path, uint64_t _size,
                  SequenceNumber _smallest_seqno,
                  SequenceNumber _largest_seqno,
                  const std::string& _smallestkey,
                  const std::string& _largestkey,
                  bool _being_compacted) :
    size(_size), name(_file_name),
    db_path(_path), smallest_seqno(_smallest_seqno), largest_seqno(_largest_seqno),
    smallestkey(_smallestkey), largestkey(_largestkey),
    being_compacted(_being_compacted) {}

  // File size in bytes.
  uint64_t size;
  // The name of the file.
  std::string name;
  // The full path where the file locates.
  std::string db_path;

  SequenceNumber smallest_seqno;  // Smallest sequence number in file.
  SequenceNumber largest_seqno;   // Largest sequence number in file.
  std::string smallestkey;     // Smallest user defined key in the file.
  std::string largestkey;      // Largest user defined key in the file.
  bool being_compacted;  // true if the file is currently being compacted.
};

// The full set of metadata associated with each SST file.
struct LiveFileMetaData : SstFileMetaData {
  std::string column_family_name;  // Name of the column family
  int level;               // Level at which this file resides.
};



}  // namespace rocksdb
#line 18 "/home/evan/source/rocksdb/include/rocksdb/db.h"
#line 1 "/home/evan/source/rocksdb/include/rocksdb/iterator.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// An iterator yields a sequence of key/value pairs from a source.
// The following class defines the interface.  Multiple implementations
// are provided by this library.  In particular, iterators are provided
// to access the contents of a Table or a DB.
//
// Multiple threads can invoke const methods on an Iterator without
// external synchronization, but if any of the threads may call a
// non-const method, all threads accessing the same Iterator must use
// external synchronization.

#ifndef STORAGE_ROCKSDB_INCLUDE_ITERATOR_H_
#define STORAGE_ROCKSDB_INCLUDE_ITERATOR_H_


namespace rocksdb {

class Iterator {
 public:
  Iterator();
  virtual ~Iterator();

  // An iterator is either positioned at a key/value pair, or
  // not valid.  This method returns true iff the iterator is valid.
  virtual bool Valid() const = 0;

  // Position at the first key in the source.  The iterator is Valid()
  // after this call iff the source is not empty.
  virtual void SeekToFirst() = 0;

  // Position at the last key in the source.  The iterator is
  // Valid() after this call iff the source is not empty.
  virtual void SeekToLast() = 0;

  // Position at the first key in the source that at or past target
  // The iterator is Valid() after this call iff the source contains
  // an entry that comes at or past target.
  virtual void Seek(const Slice& target) = 0;

  // Moves to the next entry in the source.  After this call, Valid() is
  // true iff the iterator was not positioned at the last entry in the source.
  // REQUIRES: Valid()
  virtual void Next() = 0;

  // Moves to the previous entry in the source.  After this call, Valid() is
  // true iff the iterator was not positioned at the first entry in source.
  // REQUIRES: Valid()
  virtual void Prev() = 0;

  // Return the key for the current entry.  The underlying storage for
  // the returned slice is valid only until the next modification of
  // the iterator.
  // REQUIRES: Valid()
  virtual Slice key() const = 0;

  // Return the value for the current entry.  The underlying storage for
  // the returned slice is valid only until the next modification of
  // the iterator.
  // REQUIRES: !AtEnd() && !AtStart()
  virtual Slice value() const = 0;

  // If an error has occurred, return it.  Else return an ok status.
  // If non-blocking IO is requested and this operation cannot be
  // satisfied without doing some IO, then this returns Status::Incomplete().
  virtual Status status() const = 0;

  // Clients are allowed to register function/arg1/arg2 triples that
  // will be invoked when this iterator is destroyed.
  //
  // Note that unlike all of the preceding methods, this method is
  // not abstract and therefore clients should not override it.
  typedef void (*CleanupFunction)(void* arg1, void* arg2);
  void RegisterCleanup(CleanupFunction function, void* arg1, void* arg2);

 private:
  struct Cleanup {
    CleanupFunction function;
    void* arg1;
    void* arg2;
    Cleanup* next;
  };
  Cleanup cleanup_;

  // No copying allowed
  Iterator(const Iterator&);
  void operator=(const Iterator&);
};

// Return an empty iterator (yields nothing).
extern Iterator* NewEmptyIterator();

// Return an empty iterator with the specified status.
extern Iterator* NewErrorIterator(const Status& status);

}  // namespace rocksdb

#endif  // STORAGE_ROCKSDB_INCLUDE_ITERATOR_H_
#line 20 "/home/evan/source/rocksdb/include/rocksdb/db.h"
#line 1 "/home/evan/source/rocksdb/include/rocksdb/transaction_log.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#ifndef STORAGE_ROCKSDB_INCLUDE_TRANSACTION_LOG_ITERATOR_H_
#define STORAGE_ROCKSDB_INCLUDE_TRANSACTION_LOG_ITERATOR_H_

#line 1 "/home/evan/source/rocksdb/include/rocksdb/write_batch.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// WriteBatch holds a collection of updates to apply atomically to a DB.
//
// The updates are applied in the order in which they are added
// to the WriteBatch.  For example, the value of "key" will be "v3"
// after the following batch is written:
//
//    batch.Put("key", "v1");
//    batch.Delete("key");
//    batch.Put("key", "v2");
//    batch.Put("key", "v3");
//
// Multiple threads can invoke const methods on a WriteBatch without
// external synchronization, but if any of the threads may call a
// non-const method, all threads accessing the same WriteBatch must use
// external synchronization.

#ifndef STORAGE_ROCKSDB_INCLUDE_WRITE_BATCH_H_
#define STORAGE_ROCKSDB_INCLUDE_WRITE_BATCH_H_

#include <stack>
#include <string>
#include <stdint.h>
#line 1 "/home/evan/source/rocksdb/include/rocksdb/write_batch_base.h"
// Copyright (c) 2015, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.


namespace rocksdb {

class Slice;
class Status;
class ColumnFamilyHandle;
class WriteBatch;
struct SliceParts;

// Abstract base class that defines the basic interface for a write batch.
// See WriteBatch for a basic implementation and WrithBatchWithIndex for an
// indexed implemenation.
class WriteBatchBase {
 public:
  virtual ~WriteBatchBase() {}

  // Store the mapping "key->value" in the database.
  virtual void Put(ColumnFamilyHandle* column_family, const Slice& key,
                   const Slice& value) = 0;
  virtual void Put(const Slice& key, const Slice& value) = 0;

  // Variant of Put() that gathers output like writev(2).  The key and value
  // that will be written to the database are concatentations of arrays of
  // slices.
  virtual void Put(ColumnFamilyHandle* column_family, const SliceParts& key,
                   const SliceParts& value);
  virtual void Put(const SliceParts& key, const SliceParts& value);

  // Merge "value" with the existing value of "key" in the database.
  // "key->merge(existing, value)"
  virtual void Merge(ColumnFamilyHandle* column_family, const Slice& key,
                     const Slice& value) = 0;
  virtual void Merge(const Slice& key, const Slice& value) = 0;

  // variant that takes SliceParts
  virtual void Merge(ColumnFamilyHandle* column_family, const SliceParts& key,
                     const SliceParts& value);
  virtual void Merge(const SliceParts& key, const SliceParts& value);

  // If the database contains a mapping for "key", erase it.  Else do nothing.
  virtual void Delete(ColumnFamilyHandle* column_family, const Slice& key) = 0;
  virtual void Delete(const Slice& key) = 0;

  // variant that takes SliceParts
  virtual void Delete(ColumnFamilyHandle* column_family, const SliceParts& key);
  virtual void Delete(const SliceParts& key);

  // If the database contains a mapping for "key", erase it. Expects that the
  // key was not overwritten. Else do nothing.
  virtual void SingleDelete(ColumnFamilyHandle* column_family,
                            const Slice& key) = 0;
  virtual void SingleDelete(const Slice& key) = 0;

  // variant that takes SliceParts
  virtual void SingleDelete(ColumnFamilyHandle* column_family,
                            const SliceParts& key);
  virtual void SingleDelete(const SliceParts& key);

  // Append a blob of arbitrary size to the records in this batch. The blob will
  // be stored in the transaction log but not in any other file. In particular,
  // it will not be persisted to the SST files. When iterating over this
  // WriteBatch, WriteBatch::Handler::LogData will be called with the contents
  // of the blob as it is encountered. Blobs, puts, deletes, and merges will be
  // encountered in the same order in thich they were inserted. The blob will
  // NOT consume sequence number(s) and will NOT increase the count of the batch
  //
  // Example application: add timestamps to the transaction log for use in
  // replication.
  virtual void PutLogData(const Slice& blob) = 0;

  // Clear all updates buffered in this batch.
  virtual void Clear() = 0;

  // Covert this batch into a WriteBatch.  This is an abstracted way of
  // converting any WriteBatchBase(eg WriteBatchWithIndex) into a basic
  // WriteBatch.
  virtual WriteBatch* GetWriteBatch() = 0;

  // Records the state of the batch for future calls to RollbackToSavePoint().
  // May be called multiple times to set multiple save points.
  virtual void SetSavePoint() = 0;

  // Remove all entries in this batch (Put, Merge, Delete, PutLogData) since the
  // most recent call to SetSavePoint() and removes the most recent save point.
  // If there is no previous call to SetSavePoint(), behaves the same as
  // Clear().
  virtual Status RollbackToSavePoint() = 0;
};

}  // namespace rocksdb
#line 32 "/home/evan/source/rocksdb/include/rocksdb/write_batch.h"

namespace rocksdb {

class Slice;
class ColumnFamilyHandle;
struct SavePoints;
struct SliceParts;

class WriteBatch : public WriteBatchBase {
 public:
  explicit WriteBatch(size_t reserved_bytes = 0);
  ~WriteBatch();

  using WriteBatchBase::Put;
  // Store the mapping "key->value" in the database.
  void Put(ColumnFamilyHandle* column_family, const Slice& key,
           const Slice& value) override;
  void Put(const Slice& key, const Slice& value) override {
    Put(nullptr, key, value);
  }

  // Variant of Put() that gathers output like writev(2).  The key and value
  // that will be written to the database are concatentations of arrays of
  // slices.
  void Put(ColumnFamilyHandle* column_family, const SliceParts& key,
           const SliceParts& value) override;
  void Put(const SliceParts& key, const SliceParts& value) override {
    Put(nullptr, key, value);
  }

  using WriteBatchBase::Delete;
  // If the database contains a mapping for "key", erase it.  Else do nothing.
  void Delete(ColumnFamilyHandle* column_family, const Slice& key) override;
  void Delete(const Slice& key) override { Delete(nullptr, key); }

  // variant that takes SliceParts
  void Delete(ColumnFamilyHandle* column_family,
              const SliceParts& key) override;
  void Delete(const SliceParts& key) override { Delete(nullptr, key); }

  using WriteBatchBase::SingleDelete;
  // If the database contains a mapping for "key", erase it. Expects that the
  // key was not overwritten. Else do nothing.
  void SingleDelete(ColumnFamilyHandle* column_family,
                    const Slice& key) override;
  void SingleDelete(const Slice& key) override { SingleDelete(nullptr, key); }

  // variant that takes SliceParts
  void SingleDelete(ColumnFamilyHandle* column_family,
                    const SliceParts& key) override;
  void SingleDelete(const SliceParts& key) override {
    SingleDelete(nullptr, key);
  }

  using WriteBatchBase::Merge;
  // Merge "value" with the existing value of "key" in the database.
  // "key->merge(existing, value)"
  void Merge(ColumnFamilyHandle* column_family, const Slice& key,
             const Slice& value) override;
  void Merge(const Slice& key, const Slice& value) override {
    Merge(nullptr, key, value);
  }

  // variant that takes SliceParts
  void Merge(ColumnFamilyHandle* column_family, const SliceParts& key,
             const SliceParts& value) override;
  void Merge(const SliceParts& key, const SliceParts& value) override {
    Merge(nullptr, key, value);
  }

  using WriteBatchBase::PutLogData;
  // Append a blob of arbitrary size to the records in this batch. The blob will
  // be stored in the transaction log but not in any other file. In particular,
  // it will not be persisted to the SST files. When iterating over this
  // WriteBatch, WriteBatch::Handler::LogData will be called with the contents
  // of the blob as it is encountered. Blobs, puts, deletes, and merges will be
  // encountered in the same order in thich they were inserted. The blob will
  // NOT consume sequence number(s) and will NOT increase the count of the batch
  //
  // Example application: add timestamps to the transaction log for use in
  // replication.
  void PutLogData(const Slice& blob) override;

  using WriteBatchBase::Clear;
  // Clear all updates buffered in this batch.
  void Clear() override;

  // Records the state of the batch for future calls to RollbackToSavePoint().
  // May be called multiple times to set multiple save points.
  void SetSavePoint() override;

  // Remove all entries in this batch (Put, Merge, Delete, PutLogData) since the
  // most recent call to SetSavePoint() and removes the most recent save point.
  // If there is no previous call to SetSavePoint(), Status::NotFound()
  // will be returned.
  // Oterwise returns Status::OK().
  Status RollbackToSavePoint() override;

  // Support for iterating over the contents of a batch.
  class Handler {
   public:
    virtual ~Handler();
    // default implementation will just call Put without column family for
    // backwards compatibility. If the column family is not default,
    // the function is noop
    virtual Status PutCF(uint32_t column_family_id, const Slice& key,
                         const Slice& value) {
      if (column_family_id == 0) {
        // Put() historically doesn't return status. We didn't want to be
        // backwards incompatible so we didn't change the return status
        // (this is a public API). We do an ordinary get and return Status::OK()
        Put(key, value);
        return Status::OK();
      }
      return Status::InvalidArgument(
          "non-default column family and PutCF not implemented");
    }
    virtual void Put(const Slice& key, const Slice& value) {}

    virtual Status DeleteCF(uint32_t column_family_id, const Slice& key) {
      if (column_family_id == 0) {
        Delete(key);
        return Status::OK();
      }
      return Status::InvalidArgument(
          "non-default column family and DeleteCF not implemented");
    }
    virtual void Delete(const Slice& key) {}

    virtual Status SingleDeleteCF(uint32_t column_family_id, const Slice& key) {
      if (column_family_id == 0) {
        SingleDelete(key);
        return Status::OK();
      }
      return Status::InvalidArgument(
          "non-default column family and SingleDeleteCF not implemented");
    }
    virtual void SingleDelete(const Slice& key) {}

    // Merge and LogData are not pure virtual. Otherwise, we would break
    // existing clients of Handler on a source code level. The default
    // implementation of Merge does nothing.
    virtual Status MergeCF(uint32_t column_family_id, const Slice& key,
                           const Slice& value) {
      if (column_family_id == 0) {
        Merge(key, value);
        return Status::OK();
      }
      return Status::InvalidArgument(
          "non-default column family and MergeCF not implemented");
    }
    virtual void Merge(const Slice& key, const Slice& value) {}

    // The default implementation of LogData does nothing.
    virtual void LogData(const Slice& blob);

    // Continue is called by WriteBatch::Iterate. If it returns false,
    // iteration is halted. Otherwise, it continues iterating. The default
    // implementation always returns true.
    virtual bool Continue();
  };
  Status Iterate(Handler* handler) const;

  // Retrieve the serialized version of this batch.
  const std::string& Data() const { return rep_; }

  // Retrieve data size of the batch.
  size_t GetDataSize() const { return rep_.size(); }

  // Returns the number of updates in the batch
  int Count() const;

  using WriteBatchBase::GetWriteBatch;
  WriteBatch* GetWriteBatch() override { return this; }

  // Constructor with a serialized string object
  explicit WriteBatch(const std::string& rep)
      : save_points_(nullptr), rep_(rep) {}

 private:
  friend class WriteBatchInternal;
  SavePoints* save_points_;

 protected:
  std::string rep_;  // See comment in write_batch.cc for the format of rep_

  // Intentionally copyable
};

}  // namespace rocksdb

#endif  // STORAGE_ROCKSDB_INCLUDE_WRITE_BATCH_H_
#line 11 "/home/evan/source/rocksdb/include/rocksdb/transaction_log.h"
#include <memory>
#include <vector>

namespace rocksdb {

class LogFile;
typedef std::vector<std::unique_ptr<LogFile>> VectorLogPtr;

enum  WalFileType {
  /* Indicates that WAL file is in archive directory. WAL files are moved from
   * the main db directory to archive directory once they are not live and stay
   * there until cleaned up. Files are cleaned depending on archive size
   * (Options::WAL_size_limit_MB) and time since last cleaning
   * (Options::WAL_ttl_seconds).
   */
  kArchivedLogFile = 0,

  /* Indicates that WAL file is live and resides in the main db directory */
  kAliveLogFile = 1
} ;

class LogFile {
 public:
  LogFile() {}
  virtual ~LogFile() {}

  // Returns log file's pathname relative to the main db dir
  // Eg. For a live-log-file = /000003.log
  //     For an archived-log-file = /archive/000003.log
  virtual std::string PathName() const = 0;


  // Primary identifier for log file.
  // This is directly proportional to creation time of the log file
  virtual uint64_t LogNumber() const = 0;

  // Log file can be either alive or archived
  virtual WalFileType Type() const = 0;

  // Starting sequence number of writebatch written in this log file
  virtual SequenceNumber StartSequence() const = 0;

  // Size of log file on disk in Bytes
  virtual uint64_t SizeFileBytes() const = 0;
};

struct BatchResult {
  SequenceNumber sequence = 0;
  std::unique_ptr<WriteBatch> writeBatchPtr;

  // Add empty __ctor and __dtor for the rule of five
  // However, preserve the original semantics and prohibit copying
  // as the unique_ptr member does not copy.
  BatchResult() {}

  ~BatchResult() {}

  BatchResult(const BatchResult&) = delete;

  BatchResult& operator=(const BatchResult&) = delete;

  BatchResult(BatchResult&& bResult)
      : sequence(std::move(bResult.sequence)),
        writeBatchPtr(std::move(bResult.writeBatchPtr)) {}

  BatchResult& operator=(BatchResult&& bResult) {
    sequence = std::move(bResult.sequence);
    writeBatchPtr = std::move(bResult.writeBatchPtr);
    return *this;
  }
};

// A TransactionLogIterator is used to iterate over the transactions in a db.
// One run of the iterator is continuous, i.e. the iterator will stop at the
// beginning of any gap in sequences
class TransactionLogIterator {
 public:
  TransactionLogIterator() {}
  virtual ~TransactionLogIterator() {}

  // An iterator is either positioned at a WriteBatch or not valid.
  // This method returns true if the iterator is valid.
  // Can read data from a valid iterator.
  virtual bool Valid() = 0;

  // Moves the iterator to the next WriteBatch.
  // REQUIRES: Valid() to be true.
  virtual void Next() = 0;

  // Returns ok if the iterator is valid.
  // Returns the Error when something has gone wrong.
  virtual Status status() = 0;

  // If valid return's the current write_batch and the sequence number of the
  // earliest transaction contained in the batch.
  // ONLY use if Valid() is true and status() is OK.
  virtual BatchResult GetBatch() = 0;

  // The read options for TransactionLogIterator.
  struct ReadOptions {
    // If true, all data read from underlying storage will be
    // verified against corresponding checksums.
    // Default: true
    bool verify_checksums_;

    ReadOptions() : verify_checksums_(true) {}

    explicit ReadOptions(bool verify_checksums)
        : verify_checksums_(verify_checksums) {}
  };
};
} //  namespace rocksdb

#endif  // STORAGE_ROCKSDB_INCLUDE_TRANSACTION_LOG_ITERATOR_H_
#line 23 "/home/evan/source/rocksdb/include/rocksdb/db.h"
#line 1 "/home/evan/source/rocksdb/include/rocksdb/snapshot.h"
//  Copyright (c) 2015, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.



namespace rocksdb {

class DB;

// Abstract handle to particular state of a DB.
// A Snapshot is an immutable object and can therefore be safely
// accessed from multiple threads without any external synchronization.
//
// To Create a Snapshot, call DB::GetSnapshot().
// To Destroy a Snapshot, call DB::ReleaseSnapshot(snapshot).
class Snapshot {
 public:
  // returns Snapshot's sequence number
  virtual SequenceNumber GetSequenceNumber() const = 0;

 protected:
  virtual ~Snapshot();
};

// Simple RAII wrapper class for Snapshot.
// Constructing this object will create a snapshot.  Destructing will
// release the snapshot.
class ManagedSnapshot {
 public:
  explicit ManagedSnapshot(DB* db);

  ~ManagedSnapshot();

  const Snapshot* snapshot();

 private:
  DB* db_;
  const Snapshot* snapshot_;
};

}  // namespace rocksdb
#line 25 "/home/evan/source/rocksdb/include/rocksdb/db.h"

#ifdef _WIN32
// Windows API macro interference
#undef DeleteFile
#endif


namespace rocksdb {

struct Options;
struct DBOptions;
struct ColumnFamilyOptions;
struct ReadOptions;
struct WriteOptions;
struct FlushOptions;
struct CompactionOptions;
struct CompactRangeOptions;
struct TableProperties;
struct ExternalSstFileInfo;
class WriteBatch;
class Env;
class EventListener;

using std::unique_ptr;

class ColumnFamilyHandle {
 public:
  virtual ~ColumnFamilyHandle() {}
  virtual const std::string& GetName() const = 0;
  virtual uint32_t GetID() const = 0;
};
extern const std::string kDefaultColumnFamilyName;

struct ColumnFamilyDescriptor {
  std::string name;
  ColumnFamilyOptions options;
  ColumnFamilyDescriptor()
      : name(kDefaultColumnFamilyName), options(ColumnFamilyOptions()) {}
  ColumnFamilyDescriptor(const std::string& _name,
                         const ColumnFamilyOptions& _options)
      : name(_name), options(_options) {}
};

static const int kMajorVersion = __ROCKSDB_MAJOR__;
static const int kMinorVersion = __ROCKSDB_MINOR__;

// A range of keys
struct Range {
  Slice start;          // Included in the range
  Slice limit;          // Not included in the range

  Range() { }
  Range(const Slice& s, const Slice& l) : start(s), limit(l) { }
};

// A collections of table properties objects, where
//  key: is the table's file name.
//  value: the table properties object of the given table.
typedef std::unordered_map<std::string, std::shared_ptr<const TableProperties>>
    TablePropertiesCollection;

// A DB is a persistent ordered map from keys to values.
// A DB is safe for concurrent access from multiple threads without
// any external synchronization.
class DB {
 public:
  // Open the database with the specified "name".
  // Stores a pointer to a heap-allocated database in *dbptr and returns
  // OK on success.
  // Stores nullptr in *dbptr and returns a non-OK status on error.
  // Caller should delete *dbptr when it is no longer needed.
  static Status Open(const Options& options,
                     const std::string& name,
                     DB** dbptr);

  // Open the database for read only. All DB interfaces
  // that modify data, like put/delete, will return error.
  // If the db is opened in read only mode, then no compactions
  // will happen.
  //
  // Not supported in ROCKSDB_LITE, in which case the function will
  // return Status::NotSupported.
  static Status OpenForReadOnly(const Options& options,
      const std::string& name, DB** dbptr,
      bool error_if_log_file_exist = false);

  // Open the database for read only with column families. When opening DB with
  // read only, you can specify only a subset of column families in the
  // database that should be opened. However, you always need to specify default
  // column family. The default column family name is 'default' and it's stored
  // in rocksdb::kDefaultColumnFamilyName
  //
  // Not supported in ROCKSDB_LITE, in which case the function will
  // return Status::NotSupported.
  static Status OpenForReadOnly(
      const DBOptions& db_options, const std::string& name,
      const std::vector<ColumnFamilyDescriptor>& column_families,
      std::vector<ColumnFamilyHandle*>* handles, DB** dbptr,
      bool error_if_log_file_exist = false);

  // Open DB with column families.
  // db_options specify database specific options
  // column_families is the vector of all column families in the database,
  // containing column family name and options. You need to open ALL column
  // families in the database. To get the list of column families, you can use
  // ListColumnFamilies(). Also, you can open only a subset of column families
  // for read-only access.
  // The default column family name is 'default' and it's stored
  // in rocksdb::kDefaultColumnFamilyName.
  // If everything is OK, handles will on return be the same size
  // as column_families --- handles[i] will be a handle that you
  // will use to operate on column family column_family[i]
  static Status Open(const DBOptions& db_options, const std::string& name,
                     const std::vector<ColumnFamilyDescriptor>& column_families,
                     std::vector<ColumnFamilyHandle*>* handles, DB** dbptr);

  // ListColumnFamilies will open the DB specified by argument name
  // and return the list of all column families in that DB
  // through column_families argument. The ordering of
  // column families in column_families is unspecified.
  static Status ListColumnFamilies(const DBOptions& db_options,
                                   const std::string& name,
                                   std::vector<std::string>* column_families);

  DB() { }
  virtual ~DB();

  // Create a column_family and return the handle of column family
  // through the argument handle.
  virtual Status CreateColumnFamily(const ColumnFamilyOptions& options,
                                    const std::string& column_family_name,
                                    ColumnFamilyHandle** handle);

  // Drop a column family specified by column_family handle. This call
  // only records a drop record in the manifest and prevents the column
  // family from flushing and compacting.
  virtual Status DropColumnFamily(ColumnFamilyHandle* column_family);

  // Set the database entry for "key" to "value".
  // If "key" already exists, it will be overwritten.
  // Returns OK on success, and a non-OK status on error.
  // Note: consider setting options.sync = true.
  virtual Status Put(const WriteOptions& options,
                     ColumnFamilyHandle* column_family, const Slice& key,
                     const Slice& value) = 0;
  virtual Status Put(const WriteOptions& options, const Slice& key,
                     const Slice& value) {
    return Put(options, DefaultColumnFamily(), key, value);
  }

  // Remove the database entry (if any) for "key".  Returns OK on
  // success, and a non-OK status on error.  It is not an error if "key"
  // did not exist in the database.
  // Note: consider setting options.sync = true.
  virtual Status Delete(const WriteOptions& options,
                        ColumnFamilyHandle* column_family,
                        const Slice& key) = 0;
  virtual Status Delete(const WriteOptions& options, const Slice& key) {
    return Delete(options, DefaultColumnFamily(), key);
  }

  // Remove the database entry for "key". Requires that the key exists
  // and was not overwritten. Returns OK on success, and a non-OK status
  // on error.  It is not an error if "key" did not exist in the database.
  // Note: consider setting options.sync = true.
  virtual Status SingleDelete(const WriteOptions& options,
                              ColumnFamilyHandle* column_family,
                              const Slice& key) = 0;
  virtual Status SingleDelete(const WriteOptions& options, const Slice& key) {
    return SingleDelete(options, DefaultColumnFamily(), key);
  }

  // Merge the database entry for "key" with "value".  Returns OK on success,
  // and a non-OK status on error. The semantics of this operation is
  // determined by the user provided merge_operator when opening DB.
  // Note: consider setting options.sync = true.
  virtual Status Merge(const WriteOptions& options,
                       ColumnFamilyHandle* column_family, const Slice& key,
                       const Slice& value) = 0;
  virtual Status Merge(const WriteOptions& options, const Slice& key,
                       const Slice& value) {
    return Merge(options, DefaultColumnFamily(), key, value);
  }

  // Apply the specified updates to the database.
  // If `updates` contains no update, WAL will still be synced if
  // options.sync=true.
  // Returns OK on success, non-OK on failure.
  // Note: consider setting options.sync = true.
  virtual Status Write(const WriteOptions& options, WriteBatch* updates) = 0;

  // If the database contains an entry for "key" store the
  // corresponding value in *value and return OK.
  //
  // If there is no entry for "key" leave *value unchanged and return
  // a status for which Status::IsNotFound() returns true.
  //
  // May return some other Status on an error.
  virtual Status Get(const ReadOptions& options,
                     ColumnFamilyHandle* column_family, const Slice& key,
                     std::string* value) = 0;
  virtual Status Get(const ReadOptions& options, const Slice& key, std::string* value) {
    return Get(options, DefaultColumnFamily(), key, value);
  }

  // If keys[i] does not exist in the database, then the i'th returned
  // status will be one for which Status::IsNotFound() is true, and
  // (*values)[i] will be set to some arbitrary value (often ""). Otherwise,
  // the i'th returned status will have Status::ok() true, and (*values)[i]
  // will store the value associated with keys[i].
  //
  // (*values) will always be resized to be the same size as (keys).
  // Similarly, the number of returned statuses will be the number of keys.
  // Note: keys will not be "de-duplicated". Duplicate keys will return
  // duplicate values in order.
  virtual std::vector<Status> MultiGet(
      const ReadOptions& options,
      const std::vector<ColumnFamilyHandle*>& column_family,
      const std::vector<Slice>& keys, std::vector<std::string>* values) = 0;
  virtual std::vector<Status> MultiGet(const ReadOptions& options,
                                       const std::vector<Slice>& keys,
                                       std::vector<std::string>* values) {
    return MultiGet(options, std::vector<ColumnFamilyHandle*>(
                                 keys.size(), DefaultColumnFamily()),
                    keys, values);
  }

  // If the key definitely does not exist in the database, then this method
  // returns false, else true. If the caller wants to obtain value when the key
  // is found in memory, a bool for 'value_found' must be passed. 'value_found'
  // will be true on return if value has been set properly.
  // This check is potentially lighter-weight than invoking DB::Get(). One way
  // to make this lighter weight is to avoid doing any IOs.
  // Default implementation here returns true and sets 'value_found' to false
  virtual bool KeyMayExist(const ReadOptions& options,
                           ColumnFamilyHandle* column_family, const Slice& key,
                           std::string* value, bool* value_found = nullptr) {
    if (value_found != nullptr) {
      *value_found = false;
    }
    return true;
  }
  virtual bool KeyMayExist(const ReadOptions& options, const Slice& key,
                           std::string* value, bool* value_found = nullptr) {
    return KeyMayExist(options, DefaultColumnFamily(), key, value, value_found);
  }

  // Return a heap-allocated iterator over the contents of the database.
  // The result of NewIterator() is initially invalid (caller must
  // call one of the Seek methods on the iterator before using it).
  //
  // Caller should delete the iterator when it is no longer needed.
  // The returned iterator should be deleted before this db is deleted.
  virtual Iterator* NewIterator(const ReadOptions& options,
                                ColumnFamilyHandle* column_family) = 0;
  virtual Iterator* NewIterator(const ReadOptions& options) {
    return NewIterator(options, DefaultColumnFamily());
  }
  // Returns iterators from a consistent database state across multiple
  // column families. Iterators are heap allocated and need to be deleted
  // before the db is deleted
  virtual Status NewIterators(
      const ReadOptions& options,
      const std::vector<ColumnFamilyHandle*>& column_families,
      std::vector<Iterator*>* iterators) = 0;

  // Return a handle to the current DB state.  Iterators created with
  // this handle will all observe a stable snapshot of the current DB
  // state.  The caller must call ReleaseSnapshot(result) when the
  // snapshot is no longer needed.
  //
  // nullptr will be returned if the DB fails to take a snapshot or does
  // not support snapshot.
  virtual const Snapshot* GetSnapshot() = 0;

  // Release a previously acquired snapshot.  The caller must not
  // use "snapshot" after this call.
  virtual void ReleaseSnapshot(const Snapshot* snapshot) = 0;

  // DB implementations can export properties about their state
  // via this method.  If "property" is a valid property understood by this
  // DB implementation, fills "*value" with its current value and returns
  // true.  Otherwise returns false.
  //
  //
  // Valid property names include:
  //
  //  "rocksdb.num-files-at-level<N>" - return the number of files at level <N>,
  //     where <N> is an ASCII representation of a level number (e.g. "0").
  //  "rocksdb.stats" - returns a multi-line string that describes statistics
  //     about the internal operation of the DB.
  //  "rocksdb.sstables" - returns a multi-line string that describes all
  //     of the sstables that make up the db contents.
  //  "rocksdb.cfstats"
  //  "rocksdb.dbstats"
  //  "rocksdb.num-immutable-mem-table"
  //  "rocksdb.mem-table-flush-pending"
  //  "rocksdb.compaction-pending" - 1 if at least one compaction is pending
  //  "rocksdb.background-errors" - accumulated number of background errors
  //  "rocksdb.cur-size-active-mem-table"
//  "rocksdb.size-all-mem-tables"
//  "rocksdb.num-entries-active-mem-table"
//  "rocksdb.num-entries-imm-mem-tables"
//  "rocksdb.num-deletes-active-mem-table"
//  "rocksdb.num-deletes-imm-mem-tables"
//  "rocksdb.estimate-num-keys" - estimated keys in the column family
//  "rocksdb.estimate-table-readers-mem" - estimated memory used for reding
//      SST tables, that is not counted as a part of block cache.
//  "rocksdb.is-file-deletions-enabled"
//  "rocksdb.num-snapshots"
//  "rocksdb.oldest-snapshot-time"
//  "rocksdb.num-live-versions" - `version` is an internal data structure.
//      See version_set.h for details. More live versions often mean more SST
//      files are held from being deleted, by iterators or unfinished
//      compactions.
//  "rocksdb.estimate-live-data-size"
//  "rocksdb.total-sst-files-size" - total size of all used sst files, this may
//      slow down online queries if there are too many files.
//  "rocksdb.base-level"
//  "rocksdb.estimate-pending-compaction-bytes" - estimated total number of
//      bytes compaction needs to rewrite the data to get all levels down
//      to under target size. Not valid for other compactions than level-based.
//  "rocksdb.aggregated-table-properties" - returns a string representation of
//      the aggregated table properties of the target column family.
//  "rocksdb.aggregated-table-properties-at-level<N>", same as the previous
//      one but only returns the aggregated table properties of the specified
//      level "N" at the target column family.
//  replaced by the target level.
#ifndef ROCKSDB_LITE
  struct Properties {
    static const std::string kNumFilesAtLevelPrefix;
    static const std::string kStats;
    static const std::string kSSTables;
    static const std::string kCFStats;
    static const std::string kDBStats;
    static const std::string kNumImmutableMemTable;
    static const std::string kMemTableFlushPending;
    static const std::string kCompactionPending;
    static const std::string kBackgroundErrors;
    static const std::string kCurSizeActiveMemTable;
    static const std::string kCurSizeAllMemTables;
    static const std::string kSizeAllMemTables;
    static const std::string kNumEntriesActiveMemTable;
    static const std::string kNumEntriesImmMemTables;
    static const std::string kNumDeletesActiveMemTable;
    static const std::string kNumDeletesImmMemTables;
    static const std::string kEstimateNumKeys;
    static const std::string kEstimateTableReadersMem;
    static const std::string kIsFileDeletionsEnabled;
    static const std::string kNumSnapshots;
    static const std::string kOldestSnapshotTime;
    static const std::string kNumLiveVersions;
    static const std::string kEstimateLiveDataSize;
    static const std::string kTotalSstFilesSize;
    static const std::string kEstimatePendingCompactionBytes;
    static const std::string kAggregatedTableProperties;
    static const std::string kAggregatedTablePropertiesAtLevel;
  };
#endif /* ROCKSDB_LITE */

  virtual bool GetProperty(ColumnFamilyHandle* column_family,
                           const Slice& property, std::string* value) = 0;
  virtual bool GetProperty(const Slice& property, std::string* value) {
    return GetProperty(DefaultColumnFamily(), property, value);
  }

  // Similar to GetProperty(), but only works for a subset of properties whose
  // return value is an integer. Return the value by integer. Supported
  // properties:
  //  "rocksdb.num-immutable-mem-table"
  //  "rocksdb.mem-table-flush-pending"
  //  "rocksdb.compaction-pending"
  //  "rocksdb.background-errors"
  //  "rocksdb.cur-size-active-mem-table"
  //  "rocksdb.cur-size-all-mem-tables"
  //  "rocksdb.size-all-mem-tables"
  //  "rocksdb.num-entries-active-mem-table"
  //  "rocksdb.num-entries-imm-mem-tables"
  //  "rocksdb.num-deletes-active-mem-table"
  //  "rocksdb.num-deletes-imm-mem-tables"
  //  "rocksdb.estimate-num-keys"
  //  "rocksdb.estimate-table-readers-mem"
  //  "rocksdb.is-file-deletions-enabled"
  //  "rocksdb.num-snapshots"
  //  "rocksdb.oldest-snapshot-time"
  //  "rocksdb.num-live-versions"
  //  "rocksdb.estimate-live-data-size"
  //  "rocksdb.total-sst-files-size"
  //  "rocksdb.base-level"
  //  "rocksdb.estimate-pending-compaction-bytes"
  virtual bool GetIntProperty(ColumnFamilyHandle* column_family,
                              const Slice& property, uint64_t* value) = 0;
  virtual bool GetIntProperty(const Slice& property, uint64_t* value) {
    return GetIntProperty(DefaultColumnFamily(), property, value);
  }

  // For each i in [0,n-1], store in "sizes[i]", the approximate
  // file system space used by keys in "[range[i].start .. range[i].limit)".
  //
  // Note that the returned sizes measure file system space usage, so
  // if the user data compresses by a factor of ten, the returned
  // sizes will be one-tenth the size of the corresponding user data size.
  //
  // If include_memtable is set to true, then the result will also
  // include those recently written data in the mem-tables if
  // the mem-table type supports it.
  virtual void GetApproximateSizes(ColumnFamilyHandle* column_family,
                                   const Range* range, int n, uint64_t* sizes,
                                   bool include_memtable = false) = 0;
  virtual void GetApproximateSizes(const Range* range, int n, uint64_t* sizes,
                                   bool include_memtable = false) {
    GetApproximateSizes(DefaultColumnFamily(), range, n, sizes,
                        include_memtable);
  }

  // Compact the underlying storage for the key range [*begin,*end].
  // The actual compaction interval might be superset of [*begin, *end].
  // In particular, deleted and overwritten versions are discarded,
  // and the data is rearranged to reduce the cost of operations
  // needed to access the data.  This operation should typically only
  // be invoked by users who understand the underlying implementation.
  //
  // begin==nullptr is treated as a key before all keys in the database.
  // end==nullptr is treated as a key after all keys in the database.
  // Therefore the following call will compact the entire database:
  //    db->CompactRange(options, nullptr, nullptr);
  // Note that after the entire database is compacted, all data are pushed
  // down to the last level containing any data. If the total data size after
  // compaction is reduced, that level might not be appropriate for hosting all
  // the files. In this case, client could set options.change_level to true, to
  // move the files back to the minimum level capable of holding the data set
  // or a given level (specified by non-negative options.target_level).
  virtual Status CompactRange(const CompactRangeOptions& options,
                              ColumnFamilyHandle* column_family,
                              const Slice* begin, const Slice* end) = 0;
  virtual Status CompactRange(const CompactRangeOptions& options,
                              const Slice* begin, const Slice* end) {
    return CompactRange(options, DefaultColumnFamily(), begin, end);
  }

#if defined(__GNUC__) || defined(__clang__)
  __attribute__((deprecated))
#elif _WIN32
  __declspec(deprecated)
#endif
   virtual Status
      CompactRange(ColumnFamilyHandle* column_family, const Slice* begin,
                   const Slice* end, bool change_level = false,
                   int target_level = -1, uint32_t target_path_id = 0) {
    CompactRangeOptions options;
    options.change_level = change_level;
    options.target_level = target_level;
    options.target_path_id = target_path_id;
    return CompactRange(options, column_family, begin, end);
  }
#if defined(__GNUC__) || defined(__clang__)
  __attribute__((deprecated))
#elif _WIN32
  __declspec(deprecated)
#endif
    virtual Status
      CompactRange(const Slice* begin, const Slice* end,
                   bool change_level = false, int target_level = -1,
                   uint32_t target_path_id = 0) {
    CompactRangeOptions options;
    options.change_level = change_level;
    options.target_level = target_level;
    options.target_path_id = target_path_id;
    return CompactRange(options, DefaultColumnFamily(), begin, end);
  }

  virtual Status SetOptions(ColumnFamilyHandle* column_family,
      const std::unordered_map<std::string, std::string>& new_options) {
    return Status::NotSupported("Not implemented");
  }
  virtual Status SetOptions(
      const std::unordered_map<std::string, std::string>& new_options) {
    return SetOptions(DefaultColumnFamily(), new_options);
  }

  // CompactFiles() inputs a list of files specified by file numbers and
  // compacts them to the specified level. Note that the behavior is different
  // from CompactRange() in that CompactFiles() performs the compaction job
  // using the CURRENT thread.
  //
  // @see GetDataBaseMetaData
  // @see GetColumnFamilyMetaData
  virtual Status CompactFiles(
      const CompactionOptions& compact_options,
      ColumnFamilyHandle* column_family,
      const std::vector<std::string>& input_file_names,
      const int output_level, const int output_path_id = -1) = 0;

  virtual Status CompactFiles(
      const CompactionOptions& compact_options,
      const std::vector<std::string>& input_file_names,
      const int output_level, const int output_path_id = -1) {
    return CompactFiles(compact_options, DefaultColumnFamily(),
                        input_file_names, output_level, output_path_id);
  }

  // This function will wait until all currently running background processes
  // finish. After it returns, no background process will be run until
  // UnblockBackgroundWork is called
  virtual Status PauseBackgroundWork() = 0;
  virtual Status ContinueBackgroundWork() = 0;

  // Number of levels used for this DB.
  virtual int NumberLevels(ColumnFamilyHandle* column_family) = 0;
  virtual int NumberLevels() { return NumberLevels(DefaultColumnFamily()); }

  // Maximum level to which a new compacted memtable is pushed if it
  // does not create overlap.
  virtual int MaxMemCompactionLevel(ColumnFamilyHandle* column_family) = 0;
  virtual int MaxMemCompactionLevel() {
    return MaxMemCompactionLevel(DefaultColumnFamily());
  }

  // Number of files in level-0 that would stop writes.
  virtual int Level0StopWriteTrigger(ColumnFamilyHandle* column_family) = 0;
  virtual int Level0StopWriteTrigger() {
    return Level0StopWriteTrigger(DefaultColumnFamily());
  }

  // Get DB name -- the exact same name that was provided as an argument to
  // DB::Open()
  virtual const std::string& GetName() const = 0;

  // Get Env object from the DB
  virtual Env* GetEnv() const = 0;

  // Get DB Options that we use.  During the process of opening the
  // column family, the options provided when calling DB::Open() or
  // DB::CreateColumnFamily() will have been "sanitized" and transformed
  // in an implementation-defined manner.
  virtual const Options& GetOptions(ColumnFamilyHandle* column_family)
      const = 0;
  virtual const Options& GetOptions() const {
    return GetOptions(DefaultColumnFamily());
  }

  virtual const DBOptions& GetDBOptions() const = 0;

  // Flush all mem-table data.
  virtual Status Flush(const FlushOptions& options,
                       ColumnFamilyHandle* column_family) = 0;
  virtual Status Flush(const FlushOptions& options) {
    return Flush(options, DefaultColumnFamily());
  }

  // Sync the wal. Note that Write() followed by SyncWAL() is not exactly the
  // same as Write() with sync=true: in the latter case the changes won't be
  // visible until the sync is done.
  // Currently only works if allow_mmap_writes = false in Options.
  virtual Status SyncWAL() = 0;

  // The sequence number of the most recent transaction.
  virtual SequenceNumber GetLatestSequenceNumber() const = 0;

#ifndef ROCKSDB_LITE

  // Prevent file deletions. Compactions will continue to occur,
  // but no obsolete files will be deleted. Calling this multiple
  // times have the same effect as calling it once.
  virtual Status DisableFileDeletions() = 0;

  // Allow compactions to delete obsolete files.
  // If force == true, the call to EnableFileDeletions() will guarantee that
  // file deletions are enabled after the call, even if DisableFileDeletions()
  // was called multiple times before.
  // If force == false, EnableFileDeletions will only enable file deletion
  // after it's been called at least as many times as DisableFileDeletions(),
  // enabling the two methods to be called by two threads concurrently without
  // synchronization -- i.e., file deletions will be enabled only after both
  // threads call EnableFileDeletions()
  virtual Status EnableFileDeletions(bool force = true) = 0;

  // GetLiveFiles followed by GetSortedWalFiles can generate a lossless backup

  // Retrieve the list of all files in the database. The files are
  // relative to the dbname and are not absolute paths. The valid size of the
  // manifest file is returned in manifest_file_size. The manifest file is an
  // ever growing file, but only the portion specified by manifest_file_size is
  // valid for this snapshot.
  // Setting flush_memtable to true does Flush before recording the live files.
  // Setting flush_memtable to false is useful when we don't want to wait for
  // flush which may have to wait for compaction to complete taking an
  // indeterminate time.
  //
  // In case you have multiple column families, even if flush_memtable is true,
  // you still need to call GetSortedWalFiles after GetLiveFiles to compensate
  // for new data that arrived to already-flushed column families while other
  // column families were flushing
  virtual Status GetLiveFiles(std::vector<std::string>&,
                              uint64_t* manifest_file_size,
                              bool flush_memtable = true) = 0;

  // Retrieve the sorted list of all wal files with earliest file first
  virtual Status GetSortedWalFiles(VectorLogPtr& files) = 0;

  // Sets iter to an iterator that is positioned at a write-batch containing
  // seq_number. If the sequence number is non existent, it returns an iterator
  // at the first available seq_no after the requested seq_no
  // Returns Status::OK if iterator is valid
  // Must set WAL_ttl_seconds or WAL_size_limit_MB to large values to
  // use this api, else the WAL files will get
  // cleared aggressively and the iterator might keep getting invalid before
  // an update is read.
  virtual Status GetUpdatesSince(
      SequenceNumber seq_number, unique_ptr<TransactionLogIterator>* iter,
      const TransactionLogIterator::ReadOptions&
          read_options = TransactionLogIterator::ReadOptions()) = 0;

// Windows API macro interference
#undef DeleteFile
  // Delete the file name from the db directory and update the internal state to
  // reflect that. Supports deletion of sst and log files only. 'name' must be
  // path relative to the db directory. eg. 000001.sst, /archive/000003.log
  virtual Status DeleteFile(std::string name) = 0;

  // Returns a list of all table files with their level, start key
  // and end key
  virtual void GetLiveFilesMetaData(std::vector<LiveFileMetaData>* metadata) {}

  // Obtains the meta data of the specified column family of the DB.
  // Status::NotFound() will be returned if the current DB does not have
  // any column family match the specified name.
  //
  // If cf_name is not specified, then the metadata of the default
  // column family will be returned.
  virtual void GetColumnFamilyMetaData(
      ColumnFamilyHandle* column_family,
      ColumnFamilyMetaData* metadata) {}

  // Get the metadata of the default column family.
  void GetColumnFamilyMetaData(
      ColumnFamilyMetaData* metadata) {
    GetColumnFamilyMetaData(DefaultColumnFamily(), metadata);
  }

  // Load table file located at "file_path" into "column_family", a pointer to
  // ExternalSstFileInfo can be used instead of "file_path" to do a blind add
  // that wont need to read the file, move_file can be set to true to
  // move the file instead of copying it.
  //
  // Current Requirements:
  // (1) Memtable is empty.
  // (2) All existing files (if any) have sequence number = 0.
  // (3) Key range in loaded table file don't overlap with existing
  //     files key ranges.
  // (4) No other writes happen during AddFile call, otherwise
  //     DB may get corrupted.
  // (5) Database have at least 2 levels.
  virtual Status AddFile(ColumnFamilyHandle* column_family,
                         const std::string& file_path,
                         bool move_file = false) = 0;
  virtual Status AddFile(const std::string& file_path, bool move_file = false) {
    return AddFile(DefaultColumnFamily(), file_path, move_file);
  }

  // Load table file with information "file_info" into "column_family"
  virtual Status AddFile(ColumnFamilyHandle* column_family,
                         const ExternalSstFileInfo* file_info,
                         bool move_file = false) = 0;
  virtual Status AddFile(const ExternalSstFileInfo* file_info,
                         bool move_file = false) {
    return AddFile(DefaultColumnFamily(), file_info, move_file);
  }

#endif  // ROCKSDB_LITE

  // Sets the globally unique ID created at database creation time by invoking
  // Env::GenerateUniqueId(), in identity. Returns Status::OK if identity could
  // be set properly
  virtual Status GetDbIdentity(std::string& identity) const = 0;

  // Returns default column family handle
  virtual ColumnFamilyHandle* DefaultColumnFamily() const = 0;

#ifndef ROCKSDB_LITE
  virtual Status GetPropertiesOfAllTables(ColumnFamilyHandle* column_family,
                                          TablePropertiesCollection* props) = 0;
  virtual Status GetPropertiesOfAllTables(TablePropertiesCollection* props) {
    return GetPropertiesOfAllTables(DefaultColumnFamily(), props);
  }
#endif  // ROCKSDB_LITE

  // Needed for StackableDB
  virtual DB* GetRootDB() { return this; }

 private:
  // No copying allowed
  DB(const DB&);
  void operator=(const DB&);
};

// Destroy the contents of the specified database.
// Be very careful using this method.
Status DestroyDB(const std::string& name, const Options& options);

#ifndef ROCKSDB_LITE
// If a DB cannot be opened, you may attempt to call this method to
// resurrect as much of the contents of the database as possible.
// Some data may be lost, so be careful when calling this function
// on a database that contains important information.
Status RepairDB(const std::string& dbname, const Options& options);
#endif

}  // namespace rocksdb

#endif  // STORAGE_ROCKSDB_INCLUDE_DB_H_
#line 1 "/home/evan/source/rocksdb/include/rocksdb/filter_policy.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
// Copyright (c) 2012 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// A database can be configured with a custom FilterPolicy object.
// This object is responsible for creating a small filter from a set
// of keys.  These filters are stored in rocksdb and are consulted
// automatically by rocksdb to decide whether or not to read some
// information from disk. In many cases, a filter can cut down the
// number of disk seeks form a handful to a single disk seek per
// DB::Get() call.
//
// Most people will want to use the builtin bloom filter support (see
// NewBloomFilterPolicy() below).

#ifndef STORAGE_ROCKSDB_INCLUDE_FILTER_POLICY_H_
#define STORAGE_ROCKSDB_INCLUDE_FILTER_POLICY_H_

#include <string>
#include <memory>

namespace rocksdb {

class Slice;

// A class that takes a bunch of keys, then generates filter
class FilterBitsBuilder {
 public:
  virtual ~FilterBitsBuilder() {}

  // Add Key to filter, you could use any way to store the key.
  // Such as: storing hashes or original keys
  // Keys are in sorted order and duplicated keys are possible.
  virtual void AddKey(const Slice& key) = 0;

  // Generate the filter using the keys that are added
  // The return value of this function would be the filter bits,
  // The ownership of actual data is set to buf
  virtual Slice Finish(std::unique_ptr<const char[]>* buf) = 0;
};

// A class that checks if a key can be in filter
// It should be initialized by Slice generated by BitsBuilder
class FilterBitsReader {
 public:
  virtual ~FilterBitsReader() {}

  // Check if the entry match the bits in filter
  virtual bool MayMatch(const Slice& entry) = 0;
};

// We add a new format of filter block called full filter block
// This new interface gives you more space of customization
//
// For the full filter block, you can plug in your version by implement
// the FilterBitsBuilder and FilterBitsReader
//
// There are two sets of interface in FilterPolicy
// Set 1: CreateFilter, KeyMayMatch: used for blockbased filter
// Set 2: GetFilterBitsBuilder, GetFilterBitsReader, they are used for
// full filter.
// Set 1 MUST be implemented correctly, Set 2 is optional
// RocksDB would first try using functions in Set 2. if they return nullptr,
// it would use Set 1 instead.
// You can choose filter type in NewBloomFilterPolicy
class FilterPolicy {
 public:
  virtual ~FilterPolicy();

  // Return the name of this policy.  Note that if the filter encoding
  // changes in an incompatible way, the name returned by this method
  // must be changed.  Otherwise, old incompatible filters may be
  // passed to methods of this type.
  virtual const char* Name() const = 0;

  // keys[0,n-1] contains a list of keys (potentially with duplicates)
  // that are ordered according to the user supplied comparator.
  // Append a filter that summarizes keys[0,n-1] to *dst.
  //
  // Warning: do not change the initial contents of *dst.  Instead,
  // append the newly constructed filter to *dst.
  virtual void CreateFilter(const Slice* keys, int n, std::string* dst)
      const = 0;

  // "filter" contains the data appended by a preceding call to
  // CreateFilter() on this class.  This method must return true if
  // the key was in the list of keys passed to CreateFilter().
  // This method may return true or false if the key was not on the
  // list, but it should aim to return false with a high probability.
  virtual bool KeyMayMatch(const Slice& key, const Slice& filter) const = 0;

  // Get the FilterBitsBuilder, which is ONLY used for full filter block
  // It contains interface to take individual key, then generate filter
  virtual FilterBitsBuilder* GetFilterBitsBuilder() const {
    return nullptr;
  }

  // Get the FilterBitsReader, which is ONLY used for full filter block
  // It contains interface to tell if key can be in filter
  // The input slice should NOT be deleted by FilterPolicy
  virtual FilterBitsReader* GetFilterBitsReader(const Slice& contents) const {
    return nullptr;
  }
};

// Return a new filter policy that uses a bloom filter with approximately
// the specified number of bits per key.
//
// bits_per_key: bits per key in bloom filter. A good value for bits_per_key
// is 10, which yields a filter with ~ 1% false positive rate.
// use_block_based_builder: use block based filter rather than full fiter.
// If you want to builder full filter, it needs to be set to false.
//
// Callers must delete the result after any database that is using the
// result has been closed.
//
// Note: if you are using a custom comparator that ignores some parts
// of the keys being compared, you must not use NewBloomFilterPolicy()
// and must provide your own FilterPolicy that also ignores the
// corresponding parts of the keys.  For example, if the comparator
// ignores trailing spaces, it would be incorrect to use a
// FilterPolicy (like NewBloomFilterPolicy) that does not ignore
// trailing spaces in keys.
extern const FilterPolicy* NewBloomFilterPolicy(int bits_per_key,
    bool use_block_based_builder = true);
}

#endif  // STORAGE_ROCKSDB_INCLUDE_FILTER_POLICY_H_
#line 1 "/home/evan/source/rocksdb/include/rocksdb/slice_transform.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
// Copyright (c) 2012 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// Class for specifying user-defined functions which perform a
// transformation on a slice.  It is not required that every slice
// belong to the domain and/or range of a function.  Subclasses should
// define InDomain and InRange to determine which slices are in either
// of these sets respectively.

#ifndef STORAGE_ROCKSDB_INCLUDE_SLICE_TRANSFORM_H_
#define STORAGE_ROCKSDB_INCLUDE_SLICE_TRANSFORM_H_

#include <string>

namespace rocksdb {

class Slice;

class SliceTransform {
 public:
  virtual ~SliceTransform() {};

  // Return the name of this transformation.
  virtual const char* Name() const = 0;

  // transform a src in domain to a dst in the range
  virtual Slice Transform(const Slice& src) const = 0;

  // determine whether this is a valid src upon the function applies
  virtual bool InDomain(const Slice& src) const = 0;

  // determine whether dst=Transform(src) for some src
  virtual bool InRange(const Slice& dst) const = 0;

  // Transform(s)=Transform(`prefix`) for any s with `prefix` as a prefix.
  //
  // This function is not used by RocksDB, but for users. If users pass
  // Options by string to RocksDB, they might not know what prefix extractor
  // they are using. This function is to help users can determine:
  //   if they want to iterate all keys prefixing `prefix`, whetherit is
  //   safe to use prefix bloom filter and seek to key `prefix`.
  // If this function returns true, this means a user can Seek() to a prefix
  // using the bloom filter. Otherwise, user needs to skip the bloom filter
  // by setting ReadOptions.total_order_seek = true.
  //
  // Here is an example: Suppose we implement a slice transform that returns
  // the first part of the string after spliting it using deimiter ",":
  // 1. SameResultWhenAppended("abc,") should return true. If aplying prefix
  //    bloom filter using it, all slices matching "abc:.*" will be extracted
  //    to "abc,", so any SST file or memtable containing any of those key
  //    will not be filtered out.
  // 2. SameResultWhenAppended("abc") should return false. A user will not
  //    guaranteed to see all the keys matching "abc.*" if a user seek to "abc"
  //    against a DB with the same setting. If one SST file only contains
  //    "abcd,e", the file can be filtered out and the key will be invisible.
  //
  // i.e., an implementation always returning false is safe.
  virtual bool SameResultWhenAppended(const Slice& prefix) const {
    return false;
  }
};

extern const SliceTransform* NewFixedPrefixTransform(size_t prefix_len);

extern const SliceTransform* NewCappedPrefixTransform(size_t cap_len);

extern const SliceTransform* NewNoopTransform();

}

#endif  // STORAGE_ROCKSDB_INCLUDE_SLICE_TRANSFORM_H_
#line 1 "/home/evan/source/rocksdb/include/rocksdb/table.h"
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// Currently we support two types of tables: plain table and block-based table.
//   1. Block-based table: this is the default table type that we inherited from
//      LevelDB, which was designed for storing data in hard disk or flash
//      device.
//   2. Plain table: it is one of RocksDB's SST file format optimized
//      for low query latency on pure-memory or really low-latency media.
//
// A tutorial of rocksdb table formats is available here:
//   https://github.com/facebook/rocksdb/wiki/A-Tutorial-of-RocksDB-SST-formats
//
// Example code is also available
//   https://github.com/facebook/rocksdb/wiki/A-Tutorial-of-RocksDB-SST-formats#wiki-examples

#include <memory>
#include <string>
#include <unordered_map>


namespace rocksdb {

// -- Block-based Table
class FlushBlockPolicyFactory;
class RandomAccessFile;
struct TableReaderOptions;
struct TableBuilderOptions;
class TableBuilder;
class TableReader;
class WritableFileWriter;
struct EnvOptions;
struct Options;

using std::unique_ptr;

enum ChecksumType : char {
  kNoChecksum = 0x0,  // not yet supported. Will fail
  kCRC32c = 0x1,
  kxxHash = 0x2,
};

// For advanced user only
struct BlockBasedTableOptions {
  // @flush_block_policy_factory creates the instances of flush block policy.
  // which provides a configurable way to determine when to flush a block in
  // the block based tables.  If not set, table builder will use the default
  // block flush policy, which cut blocks by block size (please refer to
  // `FlushBlockBySizePolicy`).
  std::shared_ptr<FlushBlockPolicyFactory> flush_block_policy_factory;

  // TODO(kailiu) Temporarily disable this feature by making the default value
  // to be false.
  //
  // Indicating if we'd put index/filter blocks to the block cache.
  // If not specified, each "table reader" object will pre-load index/filter
  // block during table initialization.
  bool cache_index_and_filter_blocks = false;

  // The index type that will be used for this table.
  enum IndexType : char {
    // A space efficient index block that is optimized for
    // binary-search-based index.
    kBinarySearch,

    // The hash index, if enabled, will do the hash lookup when
    // `Options.prefix_extractor` is provided.
    kHashSearch,
  };

  IndexType index_type = kBinarySearch;

  // Influence the behavior when kHashSearch is used.
  // if false, stores a precise prefix to block range mapping
  // if true, does not store prefix and allows prefix hash collision
  // (less memory consumption)
  bool hash_index_allow_collision = true;

  // Use the specified checksum type. Newly created table files will be
  // protected with this checksum type. Old table files will still be readable,
  // even though they have different checksum type.
  ChecksumType checksum = kCRC32c;

  // Disable block cache. If this is set to true,
  // then no block cache should be used, and the block_cache should
  // point to a nullptr object.
  bool no_block_cache = false;

  // If non-NULL use the specified cache for blocks.
  // If NULL, rocksdb will automatically create and use an 8MB internal cache.
  std::shared_ptr<Cache> block_cache = nullptr;

  // If non-NULL use the specified cache for compressed blocks.
  // If NULL, rocksdb will not use a compressed block cache.
  std::shared_ptr<Cache> block_cache_compressed = nullptr;

  // Approximate size of user data packed per block.  Note that the
  // block size specified here corresponds to uncompressed data.  The
  // actual size of the unit read from disk may be smaller if
  // compression is enabled.  This parameter can be changed dynamically.
  size_t block_size = 4 * 1024;

  // This is used to close a block before it reaches the configured
  // 'block_size'. If the percentage of free space in the current block is less
  // than this specified number and adding a new record to the block will
  // exceed the configured block size, then this block will be closed and the
  // new record will be written to the next block.
  int block_size_deviation = 10;

  // Number of keys between restart points for delta encoding of keys.
  // This parameter can be changed dynamically.  Most clients should
  // leave this parameter alone.
  int block_restart_interval = 16;

  // If non-nullptr, use the specified filter policy to reduce disk reads.
  // Many applications will benefit from passing the result of
  // NewBloomFilterPolicy() here.
  std::shared_ptr<const FilterPolicy> filter_policy = nullptr;

  // If true, place whole keys in the filter (not just prefixes).
  // This must generally be true for gets to be efficient.
  bool whole_key_filtering = true;

  // We currently have three versions:
  // 0 -- This version is currently written out by all RocksDB's versions by
  // default.  Can be read by really old RocksDB's. Doesn't support changing
  // checksum (default is CRC32).
  // 1 -- Can be read by RocksDB's versions since 3.0. Supports non-default
  // checksum, like xxHash. It is written by RocksDB when
  // BlockBasedTableOptions::checksum is something other than kCRC32c. (version
  // 0 is silently upconverted)
  // 2 -- Can be read by RocksDB's versions since 3.10. Changes the way we
  // encode compressed blocks with LZ4, BZip2 and Zlib compression. If you
  // don't plan to run RocksDB before version 3.10, you should probably use
  // this.
  // This option only affects newly written tables. When reading exising tables,
  // the information about version is read from the footer.
  uint32_t format_version = 0;
};

// Table Properties that are specific to block-based table properties.
struct BlockBasedTablePropertyNames {
  // value of this propertis is a fixed int32 number.
  static const std::string kIndexType;
  // value is "1" for true and "0" for false.
  static const std::string kWholeKeyFiltering;
  // value is "1" for true and "0" for false.
  static const std::string kPrefixFiltering;
};

// Create default block based table factory.
extern TableFactory* NewBlockBasedTableFactory(
    const BlockBasedTableOptions& table_options = BlockBasedTableOptions());

#ifndef ROCKSDB_LITE

enum EncodingType : char {
  // Always write full keys without any special encoding.
  kPlain,
  // Find opportunity to write the same prefix once for multiple rows.
  // In some cases, when a key follows a previous key with the same prefix,
  // instead of writing out the full key, it just writes out the size of the
  // shared prefix, as well as other bytes, to save some bytes.
  //
  // When using this option, the user is required to use the same prefix
  // extractor to make sure the same prefix will be extracted from the same key.
  // The Name() value of the prefix extractor will be stored in the file. When
  // reopening the file, the name of the options.prefix_extractor given will be
  // bitwise compared to the prefix extractors stored in the file. An error
  // will be returned if the two don't match.
  kPrefix,
};

// Table Properties that are specific to plain table properties.
struct PlainTablePropertyNames {
  static const std::string kPrefixExtractorName;
  static const std::string kEncodingType;
  static const std::string kBloomVersion;
  static const std::string kNumBloomBlocks;
};

const uint32_t kPlainTableVariableLength = 0;

struct PlainTableOptions {
  // @user_key_len: plain table has optimization for fix-sized keys, which can
  //                be specified via user_key_len.  Alternatively, you can pass
  //                `kPlainTableVariableLength` if your keys have variable
  //                lengths.
  uint32_t user_key_len = kPlainTableVariableLength;

  // @bloom_bits_per_key: the number of bits used for bloom filer per prefix.
  //                      You may disable it by passing a zero.
  int bloom_bits_per_key = 10;

  // @hash_table_ratio: the desired utilization of the hash table used for
  //                    prefix hashing.
  //                    hash_table_ratio = number of prefixes / #buckets in the
  //                    hash table
  double hash_table_ratio = 0.75;

  // @index_sparseness: inside each prefix, need to build one index record for
  //                    how many keys for binary search inside each hash bucket.
  //                    For encoding type kPrefix, the value will be used when
  //                    writing to determine an interval to rewrite the full
  //                    key. It will also be used as a suggestion and satisfied
  //                    when possible.
  size_t index_sparseness = 16;

  // @huge_page_tlb_size: if <=0, allocate hash indexes and blooms from malloc.
  //                      Otherwise from huge page TLB. The user needs to
  //                      reserve huge pages for it to be allocated, like:
  //                          sysctl -w vm.nr_hugepages=20
  //                      See linux doc Documentation/vm/hugetlbpage.txt
  size_t huge_page_tlb_size = 0;

  // @encoding_type: how to encode the keys. See enum EncodingType above for
  //                 the choices. The value will determine how to encode keys
  //                 when writing to a new SST file. This value will be stored
  //                 inside the SST file which will be used when reading from
  //                 the file, which makes it possible for users to choose
  //                 different encoding type when reopening a DB. Files with
  //                 different encoding types can co-exist in the same DB and
  //                 can be read.
  EncodingType encoding_type = kPlain;

  // @full_scan_mode: mode for reading the whole file one record by one without
  //                  using the index.
  bool full_scan_mode = false;

  // @store_index_in_file: compute plain table index and bloom filter during
  //                       file building and store it in file. When reading
  //                       file, index will be mmaped instead of recomputation.
  bool store_index_in_file = false;
};

// -- Plain Table with prefix-only seek
// For this factory, you need to set Options.prefix_extrator properly to make it
// work. Look-up will starts with prefix hash lookup for key prefix. Inside the
// hash bucket found, a binary search is executed for hash conflicts. Finally,
// a linear search is used.

extern TableFactory* NewPlainTableFactory(const PlainTableOptions& options =
                                              PlainTableOptions());

struct CuckooTablePropertyNames {
  // The key that is used to fill empty buckets.
  static const std::string kEmptyKey;
  // Fixed length of value.
  static const std::string kValueLength;
  // Number of hash functions used in Cuckoo Hash.
  static const std::string kNumHashFunc;
  // It denotes the number of buckets in a Cuckoo Block. Given a key and a
  // particular hash function, a Cuckoo Block is a set of consecutive buckets,
  // where starting bucket id is given by the hash function on the key. In case
  // of a collision during inserting the key, the builder tries to insert the
  // key in other locations of the cuckoo block before using the next hash
  // function. This reduces cache miss during read operation in case of
  // collision.
  static const std::string kCuckooBlockSize;
  // Size of the hash table. Use this number to compute the modulo of hash
  // function. The actual number of buckets will be kMaxHashTableSize +
  // kCuckooBlockSize - 1. The last kCuckooBlockSize-1 buckets are used to
  // accommodate the Cuckoo Block from end of hash table, due to cache friendly
  // implementation.
  static const std::string kHashTableSize;
  // Denotes if the key sorted in the file is Internal Key (if false)
  // or User Key only (if true).
  static const std::string kIsLastLevel;
  // Indicate if using identity function for the first hash function.
  static const std::string kIdentityAsFirstHash;
  // Indicate if using module or bit and to calculate hash value
  static const std::string kUseModuleHash;
  // Fixed user key length
  static const std::string kUserKeyLength;
};

struct CuckooTableOptions {
  // Determines the utilization of hash tables. Smaller values
  // result in larger hash tables with fewer collisions.
  double hash_table_ratio = 0.9;
  // A property used by builder to determine the depth to go to
  // to search for a path to displace elements in case of
  // collision. See Builder.MakeSpaceForKey method. Higher
  // values result in more efficient hash tables with fewer
  // lookups but take more time to build.
  uint32_t max_search_depth = 100;
  // In case of collision while inserting, the builder
  // attempts to insert in the next cuckoo_block_size
  // locations before skipping over to the next Cuckoo hash
  // function. This makes lookups more cache friendly in case
  // of collisions.
  uint32_t cuckoo_block_size = 5;
  // If this option is enabled, user key is treated as uint64_t and its value
  // is used as hash value directly. This option changes builder's behavior.
  // Reader ignore this option and behave according to what specified in table
  // property.
  bool identity_as_first_hash = false;
  // If this option is set to true, module is used during hash calculation.
  // This often yields better space efficiency at the cost of performance.
  // If this optino is set to false, # of entries in table is constrained to be
  // power of two, and bit and is used to calculate hash, which is faster in
  // general.
  bool use_module_hash = true;
};

// Cuckoo Table Factory for SST table format using Cache Friendly Cuckoo Hashing
extern TableFactory* NewCuckooTableFactory(
    const CuckooTableOptions& table_options = CuckooTableOptions());

#endif  // ROCKSDB_LITE

class RandomAccessFileReader;

// A base class for table factories.
class TableFactory {
 public:
  virtual ~TableFactory() {}

  // The type of the table.
  //
  // The client of this package should switch to a new name whenever
  // the table format implementation changes.
  //
  // Names starting with "rocksdb." are reserved and should not be used
  // by any clients of this package.
  virtual const char* Name() const = 0;

  // Returns a Table object table that can fetch data from file specified
  // in parameter file. It's the caller's responsibility to make sure
  // file is in the correct format.
  //
  // NewTableReader() is called in three places:
  // (1) TableCache::FindTable() calls the function when table cache miss
  //     and cache the table object returned.
  // (2) SstFileReader (for SST Dump) opens the table and dump the table
  //     contents using the interator of the table.
  // (3) DBImpl::AddFile() calls this function to read the contents of
  //     the sst file it's attempting to add
  //
  // table_reader_options is a TableReaderOptions which contain all the
  //    needed parameters and configuration to open the table.
  // file is a file handler to handle the file for the table.
  // file_size is the physical file size of the file.
  // table_reader is the output table reader.
  virtual Status NewTableReader(
      const TableReaderOptions& table_reader_options,
      unique_ptr<RandomAccessFileReader>&& file, uint64_t file_size,
      unique_ptr<TableReader>* table_reader) const = 0;

  // Return a table builder to write to a file for this table type.
  //
  // It is called in several places:
  // (1) When flushing memtable to a level-0 output file, it creates a table
  //     builder (In DBImpl::WriteLevel0Table(), by calling BuildTable())
  // (2) During compaction, it gets the builder for writing compaction output
  //     files in DBImpl::OpenCompactionOutputFile().
  // (3) When recovering from transaction logs, it creates a table builder to
  //     write to a level-0 output file (In DBImpl::WriteLevel0TableForRecovery,
  //     by calling BuildTable())
  // (4) When running Repairer, it creates a table builder to convert logs to
  //     SST files (In Repairer::ConvertLogToTable() by calling BuildTable())
  //
  // ImmutableCFOptions is a subset of Options that can not be altered.
  // Multiple configured can be acceseed from there, including and not limited
  // to compression options. file is a handle of a writable file.
  // It is the caller's responsibility to keep the file open and close the file
  // after closing the table builder. compression_type is the compression type
  // to use in this table.
  virtual TableBuilder* NewTableBuilder(
      const TableBuilderOptions& table_builder_options,
      WritableFileWriter* file) const = 0;

  // Sanitizes the specified DB Options and ColumnFamilyOptions.
  //
  // If the function cannot find a way to sanitize the input DB Options,
  // a non-ok Status will be returned.
  virtual Status SanitizeOptions(
      const DBOptions& db_opts,
      const ColumnFamilyOptions& cf_opts) const = 0;

  // Return a string that contains printable format of table configurations.
  // RocksDB prints configurations at DB Open().
  virtual std::string GetPrintableTableOptions() const = 0;
};

#ifndef ROCKSDB_LITE
// Create a special table factory that can open either of the supported
// table formats, based on setting inside the SST files. It should be used to
// convert a DB from one table format to another.
// @table_factory_to_write: the table factory used when writing to new files.
// @block_based_table_factory:  block based table factory to use. If NULL, use
//                              a default one.
// @plain_table_factory: plain table factory to use. If NULL, use a default one.
// @cuckoo_table_factory: cuckoo table factory to use. If NULL, use a default one.
extern TableFactory* NewAdaptiveTableFactory(
    std::shared_ptr<TableFactory> table_factory_to_write = nullptr,
    std::shared_ptr<TableFactory> block_based_table_factory = nullptr,
    std::shared_ptr<TableFactory> plain_table_factory = nullptr,
    std::shared_ptr<TableFactory> cuckoo_table_factory = nullptr);

#endif  // ROCKSDB_LITE

}  // namespace rocksdb
#line 1 "/home/evan/source/rocksdb/include/rocksdb/cache.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// A Cache is an interface that maps keys to values.  It has internal
// synchronization and may be safely accessed concurrently from
// multiple threads.  It may automatically evict entries to make room
// for new entries.  Values have a specified charge against the cache
// capacity.  For example, a cache where the values are variable
// length strings, may use the length of the string as the charge for
// the string.
//
// A builtin cache implementation with a least-recently-used eviction
// policy is provided.  Clients may use their own implementations if
// they want something more sophisticated (like scan-resistance, a
// custom eviction policy, variable cache sizing, etc.)

#ifndef STORAGE_ROCKSDB_INCLUDE_CACHE_H_
#define STORAGE_ROCKSDB_INCLUDE_CACHE_H_

#include <memory>
#include <stdint.h>

namespace rocksdb {

using std::shared_ptr;

class Cache;

// Create a new cache with a fixed size capacity. The cache is sharded
// to 2^numShardBits shards, by hash of the key. The total capacity
// is divided and evenly assigned to each shard.
//
// The functions without parameter numShardBits uses default value, which is 4
extern shared_ptr<Cache> NewLRUCache(size_t capacity);
extern shared_ptr<Cache> NewLRUCache(size_t capacity, int numShardBits);

class Cache {
 public:
  Cache() { }

  // Destroys all existing entries by calling the "deleter"
  // function that was passed to the constructor.
  virtual ~Cache();

  // Opaque handle to an entry stored in the cache.
  struct Handle { };

  // Insert a mapping from key->value into the cache and assign it
  // the specified charge against the total cache capacity.
  //
  // Returns a handle that corresponds to the mapping.  The caller
  // must call this->Release(handle) when the returned mapping is no
  // longer needed.
  //
  // When the inserted entry is no longer needed, the key and
  // value will be passed to "deleter".
  virtual Handle* Insert(const Slice& key, void* value, size_t charge,
                         void (*deleter)(const Slice& key, void* value)) = 0;

  // If the cache has no mapping for "key", returns nullptr.
  //
  // Else return a handle that corresponds to the mapping.  The caller
  // must call this->Release(handle) when the returned mapping is no
  // longer needed.
  virtual Handle* Lookup(const Slice& key) = 0;

  // Release a mapping returned by a previous Lookup().
  // REQUIRES: handle must not have been released yet.
  // REQUIRES: handle must have been returned by a method on *this.
  virtual void Release(Handle* handle) = 0;

  // Return the value encapsulated in a handle returned by a
  // successful Lookup().
  // REQUIRES: handle must not have been released yet.
  // REQUIRES: handle must have been returned by a method on *this.
  virtual void* Value(Handle* handle) = 0;

  // If the cache contains entry for key, erase it.  Note that the
  // underlying entry will be kept around until all existing handles
  // to it have been released.
  virtual void Erase(const Slice& key) = 0;

  // Return a new numeric id.  May be used by multiple clients who are
  // sharing the same cache to partition the key space.  Typically the
  // client will allocate a new id at startup and prepend the id to
  // its cache keys.
  virtual uint64_t NewId() = 0;

  // sets the maximum configured capacity of the cache. When the new
  // capacity is less than the old capacity and the existing usage is
  // greater than new capacity, the implementation will do its best job to
  // purge the released entries from the cache in order to lower the usage
  virtual void SetCapacity(size_t capacity) = 0;

  // returns the maximum configured capacity of the cache
  virtual size_t GetCapacity() const = 0;

  // returns the memory size for the entries residing in the cache.
  virtual size_t GetUsage() const = 0;

  // returns the memory size for a specific entry in the cache.
  virtual size_t GetUsage(Handle* handle) const = 0;

  // returns the memory size for the entries in use by the system
  virtual size_t GetPinnedUsage() const = 0;

  // Call this on shutdown if you want to speed it up. Cache will disown
  // any underlying data and will not free it on delete. This call will leak
  // memory - call this only if you're shutting down the process.
  // Any attempts of using cache after this call will fail terribly.
  // Always delete the DB object before calling this method!
  virtual void DisownData() {
    // default implementation is noop
  };

  // Apply callback to all entries in the cache
  // If thread_safe is true, it will also lock the accesses. Otherwise, it will
  // access the cache without the lock held
  virtual void ApplyToAllCacheEntries(void (*callback)(void*, size_t),
                                      bool thread_safe) = 0;

 private:
  void LRU_Remove(Handle* e);
  void LRU_Append(Handle* e);
  void Unref(Handle* e);

  // No copying allowed
  Cache(const Cache&);
  void operator=(const Cache&);
};

}  // namespace rocksdb

#endif  // STORAGE_ROCKSDB_UTIL_CACHE_H_
#line 1 "/home/evan/source/rocksdb/include/rocksdb/memtablerep.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
//
// This file contains the interface that must be implemented by any collection
// to be used as the backing store for a MemTable. Such a collection must
// satisfy the following properties:
//  (1) It does not store duplicate items.
//  (2) It uses MemTableRep::KeyComparator to compare items for iteration and
//     equality.
//  (3) It can be accessed concurrently by multiple readers and can support
//     during reads. However, it needn't support multiple concurrent writes.
//  (4) Items are never deleted.
// The liberal use of assertions is encouraged to enforce (1).
//
// The factory will be passed an MemTableAllocator object when a new MemTableRep
// is requested.
//
// Users can implement their own memtable representations. We include three
// types built in:
//  - SkipListRep: This is the default; it is backed by a skip list.
//  - HashSkipListRep: The memtable rep that is best used for keys that are
//  structured like "prefix:suffix" where iteration within a prefix is
//  common and iteration across different prefixes is rare. It is backed by
//  a hash map where each bucket is a skip list.
//  - VectorRep: This is backed by an unordered std::vector. On iteration, the
// vector is sorted. It is intelligent about sorting; once the MarkReadOnly()
// has been called, the vector will only be sorted once. It is optimized for
// random-write-heavy workloads.
//
// The last four implementations are designed for situations in which
// iteration over the entire collection is rare since doing so requires all the
// keys to be copied into a sorted data structure.


#include <memory>
#include <stdint.h>

namespace rocksdb {

class Arena;
class MemTableAllocator;
class LookupKey;
class Slice;
class SliceTransform;
class Logger;

typedef void* KeyHandle;

class MemTableRep {
 public:
  // KeyComparator provides a means to compare keys, which are internal keys
  // concatenated with values.
  class KeyComparator {
   public:
    // Compare a and b. Return a negative value if a is less than b, 0 if they
    // are equal, and a positive value if a is greater than b
    virtual int operator()(const char* prefix_len_key1,
                           const char* prefix_len_key2) const = 0;

    virtual int operator()(const char* prefix_len_key,
                           const Slice& key) const = 0;

    virtual ~KeyComparator() { }
  };

  explicit MemTableRep(MemTableAllocator* allocator) : allocator_(allocator) {}

  // Allocate a buf of len size for storing key. The idea is that a specific
  // memtable representation knows its underlying data structure better. By
  // allowing it to allocate memory, it can possibly put correlated stuff
  // in consecutive memory area to make processor prefetching more efficient.
  virtual KeyHandle Allocate(const size_t len, char** buf);

  // Insert key into the collection. (The caller will pack key and value into a
  // single buffer and pass that in as the parameter to Insert).
  // REQUIRES: nothing that compares equal to key is currently in the
  // collection.
  virtual void Insert(KeyHandle handle) = 0;

  // Returns true iff an entry that compares equal to key is in the collection.
  virtual bool Contains(const char* key) const = 0;

  // Notify this table rep that it will no longer be added to. By default, does
  // nothing.  After MarkReadOnly() is called, this table rep will not be
  // written to (ie No more calls to Allocate(), Insert(), or any writes done
  // directly to entries accessed through the iterator.)
  virtual void MarkReadOnly() { }

  // Look up key from the mem table, since the first key in the mem table whose
  // user_key matches the one given k, call the function callback_func(), with
  // callback_args directly forwarded as the first parameter, and the mem table
  // key as the second parameter. If the return value is false, then terminates.
  // Otherwise, go through the next key.
  // It's safe for Get() to terminate after having finished all the potential
  // key for the k.user_key(), or not.
  //
  // Default:
  // Get() function with a default value of dynamically construct an iterator,
  // seek and call the call back function.
  virtual void Get(const LookupKey& k, void* callback_args,
                   bool (*callback_func)(void* arg, const char* entry));

  virtual uint64_t ApproximateNumEntries(const Slice& start_ikey,
                                         const Slice& end_key) {
    return 0;
  }

  // Report an approximation of how much memory has been used other than memory
  // that was allocated through the allocator.
  virtual size_t ApproximateMemoryUsage() = 0;

  virtual ~MemTableRep() { }

  // Iteration over the contents of a skip collection
  class Iterator {
   public:
    // Initialize an iterator over the specified collection.
    // The returned iterator is not valid.
    // explicit Iterator(const MemTableRep* collection);
    virtual ~Iterator() {}

    // Returns true iff the iterator is positioned at a valid node.
    virtual bool Valid() const = 0;

    // Returns the key at the current position.
    // REQUIRES: Valid()
    virtual const char* key() const = 0;

    // Advances to the next position.
    // REQUIRES: Valid()
    virtual void Next() = 0;

    // Advances to the previous position.
    // REQUIRES: Valid()
    virtual void Prev() = 0;

    // Advance to the first entry with a key >= target
    virtual void Seek(const Slice& internal_key, const char* memtable_key) = 0;

    // Position at the first entry in collection.
    // Final state of iterator is Valid() iff collection is not empty.
    virtual void SeekToFirst() = 0;

    // Position at the last entry in collection.
    // Final state of iterator is Valid() iff collection is not empty.
    virtual void SeekToLast() = 0;
  };

  // Return an iterator over the keys in this representation.
  // arena: If not null, the arena needs to be used to allocate the Iterator.
  //        When destroying the iterator, the caller will not call "delete"
  //        but Iterator::~Iterator() directly. The destructor needs to destroy
  //        all the states but those allocated in arena.
  virtual Iterator* GetIterator(Arena* arena = nullptr) = 0;

  // Return an iterator that has a special Seek semantics. The result of
  // a Seek might only include keys with the same prefix as the target key.
  // arena: If not null, the arena is used to allocate the Iterator.
  //        When destroying the iterator, the caller will not call "delete"
  //        but Iterator::~Iterator() directly. The destructor needs to destroy
  //        all the states but those allocated in arena.
  virtual Iterator* GetDynamicPrefixIterator(Arena* arena = nullptr) {
    return GetIterator(arena);
  }

  // Return true if the current MemTableRep supports merge operator.
  // Default: true
  virtual bool IsMergeOperatorSupported() const { return true; }

  // Return true if the current MemTableRep supports snapshot
  // Default: true
  virtual bool IsSnapshotSupported() const { return true; }

 protected:
  // When *key is an internal key concatenated with the value, returns the
  // user key.
  virtual Slice UserKey(const char* key) const;

  MemTableAllocator* allocator_;
};

// This is the base class for all factories that are used by RocksDB to create
// new MemTableRep objects
class MemTableRepFactory {
 public:
  virtual ~MemTableRepFactory() {}
  virtual MemTableRep* CreateMemTableRep(const MemTableRep::KeyComparator&,
                                         MemTableAllocator*,
                                         const SliceTransform*,
                                         Logger* logger) = 0;
  virtual const char* Name() const = 0;
};

// This uses a skip list to store keys. It is the default.
//
// Parameters:
//   lookahead: If non-zero, each iterator's seek operation will start the
//     search from the previously visited record (doing at most 'lookahead'
//     steps). This is an optimization for the access pattern including many
//     seeks with consecutive keys.
class SkipListFactory : public MemTableRepFactory {
 public:
  explicit SkipListFactory(size_t lookahead = 0) : lookahead_(lookahead) {}

  virtual MemTableRep* CreateMemTableRep(const MemTableRep::KeyComparator&,
                                         MemTableAllocator*,
                                         const SliceTransform*,
                                         Logger* logger) override;
  virtual const char* Name() const override { return "SkipListFactory"; }

 private:
  const size_t lookahead_;
};

#ifndef ROCKSDB_LITE
// This creates MemTableReps that are backed by an std::vector. On iteration,
// the vector is sorted. This is useful for workloads where iteration is very
// rare and writes are generally not issued after reads begin.
//
// Parameters:
//   count: Passed to the constructor of the underlying std::vector of each
//     VectorRep. On initialization, the underlying array will be at least count
//     bytes reserved for usage.
class VectorRepFactory : public MemTableRepFactory {
  const size_t count_;

 public:
  explicit VectorRepFactory(size_t count = 0) : count_(count) { }
  virtual MemTableRep* CreateMemTableRep(const MemTableRep::KeyComparator&,
                                         MemTableAllocator*,
                                         const SliceTransform*,
                                         Logger* logger) override;
  virtual const char* Name() const override {
    return "VectorRepFactory";
  }
};

// This class contains a fixed array of buckets, each
// pointing to a skiplist (null if the bucket is empty).
// bucket_count: number of fixed array buckets
// skiplist_height: the max height of the skiplist
// skiplist_branching_factor: probabilistic size ratio between adjacent
//                            link lists in the skiplist
extern MemTableRepFactory* NewHashSkipListRepFactory(
    size_t bucket_count = 1000000, int32_t skiplist_height = 4,
    int32_t skiplist_branching_factor = 4
);

// The factory is to create memtables based on a hash table:
// it contains a fixed array of buckets, each pointing to either a linked list
// or a skip list if number of entries inside the bucket exceeds
// threshold_use_skiplist.
// @bucket_count: number of fixed array buckets
// @huge_page_tlb_size: if <=0, allocate the hash table bytes from malloc.
//                      Otherwise from huge page TLB. The user needs to reserve
//                      huge pages for it to be allocated, like:
//                          sysctl -w vm.nr_hugepages=20
//                      See linux doc Documentation/vm/hugetlbpage.txt
// @bucket_entries_logging_threshold: if number of entries in one bucket
//                                    exceeds this number, log about it.
// @if_log_bucket_dist_when_flash: if true, log distribution of number of
//                                 entries when flushing.
// @threshold_use_skiplist: a bucket switches to skip list if number of
//                          entries exceed this parameter.
extern MemTableRepFactory* NewHashLinkListRepFactory(
    size_t bucket_count = 50000, size_t huge_page_tlb_size = 0,
    int bucket_entries_logging_threshold = 4096,
    bool if_log_bucket_dist_when_flash = true,
    uint32_t threshold_use_skiplist = 256);

// This factory creates a cuckoo-hashing based mem-table representation.
// Cuckoo-hash is a closed-hash strategy, in which all key/value pairs
// are stored in the bucket array itself intead of in some data structures
// external to the bucket array.  In addition, each key in cuckoo hash
// has a constant number of possible buckets in the bucket array.  These
// two properties together makes cuckoo hash more memory efficient and
// a constant worst-case read time.  Cuckoo hash is best suitable for
// point-lookup workload.
//
// When inserting a key / value, it first checks whether one of its possible
// buckets is empty.  If so, the key / value will be inserted to that vacant
// bucket.  Otherwise, one of the keys originally stored in one of these
// possible buckets will be "kicked out" and move to one of its possible
// buckets (and possibly kicks out another victim.)  In the current
// implementation, such "kick-out" path is bounded.  If it cannot find a
// "kick-out" path for a specific key, this key will be stored in a backup
// structure, and the current memtable to be forced to immutable.
//
// Note that currently this mem-table representation does not support
// snapshot (i.e., it only queries latest state) and iterators.  In addition,
// MultiGet operation might also lose its atomicity due to the lack of
// snapshot support.
//
// Parameters:
//   write_buffer_size: the write buffer size in bytes.
//   average_data_size: the average size of key + value in bytes.  This value
//     together with write_buffer_size will be used to compute the number
//     of buckets.
//   hash_function_count: the number of hash functions that will be used by
//     the cuckoo-hash.  The number also equals to the number of possible
//     buckets each key will have.
extern MemTableRepFactory* NewHashCuckooRepFactory(
    size_t write_buffer_size, size_t average_data_size = 64,
    unsigned int hash_function_count = 4);
#endif  // ROCKSDB_LITE
}  // namespace rocksdb
#line 1 "/home/evan/source/rocksdb/include/rocksdb/statistics.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#ifndef STORAGE_ROCKSDB_INCLUDE_STATISTICS_H_
#define STORAGE_ROCKSDB_INCLUDE_STATISTICS_H_

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string>
#include <memory>
#include <vector>

namespace rocksdb {

/**
 * Keep adding ticker's here.
 *  1. Any ticker should be added before TICKER_ENUM_MAX.
 *  2. Add a readable string in TickersNameMap below for the newly added ticker.
 */
enum Tickers : uint32_t {
  // total block cache misses
  // REQUIRES: BLOCK_CACHE_MISS == BLOCK_CACHE_INDEX_MISS +
  //                               BLOCK_CACHE_FILTER_MISS +
  //                               BLOCK_CACHE_DATA_MISS;
  BLOCK_CACHE_MISS = 0,
  // total block cache hit
  // REQUIRES: BLOCK_CACHE_HIT == BLOCK_CACHE_INDEX_HIT +
  //                              BLOCK_CACHE_FILTER_HIT +
  //                              BLOCK_CACHE_DATA_HIT;
  BLOCK_CACHE_HIT,
  // # of blocks added to block cache.
  BLOCK_CACHE_ADD,
  // # of times cache miss when accessing index block from block cache.
  BLOCK_CACHE_INDEX_MISS,
  // # of times cache hit when accessing index block from block cache.
  BLOCK_CACHE_INDEX_HIT,
  // # of times cache miss when accessing filter block from block cache.
  BLOCK_CACHE_FILTER_MISS,
  // # of times cache hit when accessing filter block from block cache.
  BLOCK_CACHE_FILTER_HIT,
  // # of times cache miss when accessing data block from block cache.
  BLOCK_CACHE_DATA_MISS,
  // # of times cache hit when accessing data block from block cache.
  BLOCK_CACHE_DATA_HIT,
  // # of bytes read from cache.
  BLOCK_CACHE_BYTES_READ,
  // # of bytes written into cache.
  BLOCK_CACHE_BYTES_WRITE,
  // # of times bloom filter has avoided file reads.
  BLOOM_FILTER_USEFUL,

  // # of memtable hits.
  MEMTABLE_HIT,
  // # of memtable misses.
  MEMTABLE_MISS,

  // # of Get() queries served by L0
  GET_HIT_L0,
  // # of Get() queries served by L1
  GET_HIT_L1,
  // # of Get() queries served by L2 and up
  GET_HIT_L2_AND_UP,

  /**
   * COMPACTION_KEY_DROP_* count the reasons for key drop during compaction
   * There are 3 reasons currently.
   */
  COMPACTION_KEY_DROP_NEWER_ENTRY,  // key was written with a newer value.
  COMPACTION_KEY_DROP_OBSOLETE,     // The key is obsolete.
  COMPACTION_KEY_DROP_USER,  // user compaction function has dropped the key.

  // Number of keys written to the database via the Put and Write call's
  NUMBER_KEYS_WRITTEN,
  // Number of Keys read,
  NUMBER_KEYS_READ,
  // Number keys updated, if inplace update is enabled
  NUMBER_KEYS_UPDATED,
  // The number of uncompressed bytes issued by DB::Put(), DB::Delete(),
  // DB::Merge(), and DB::Write().
  BYTES_WRITTEN,
  // The number of uncompressed bytes read from DB::Get().  It could be
  // either from memtables, cache, or table files.
  // For the number of logical bytes read from DB::MultiGet(),
  // please use NUMBER_MULTIGET_BYTES_READ.
  BYTES_READ,
  // The number of calls to seek/next/prev
  NUMBER_DB_SEEK,
  NUMBER_DB_NEXT,
  NUMBER_DB_PREV,
  // The number of calls to seek/next/prev that returned data
  NUMBER_DB_SEEK_FOUND,
  NUMBER_DB_NEXT_FOUND,
  NUMBER_DB_PREV_FOUND,
  // The number of uncompressed bytes read from an iterator.
  // Includes size of key and value.
  ITER_BYTES_READ,
  NO_FILE_CLOSES,
  NO_FILE_OPENS,
  NO_FILE_ERRORS,
  // DEPRECATED Time system had to wait to do LO-L1 compactions
  STALL_L0_SLOWDOWN_MICROS,
  // DEPRECATED Time system had to wait to move memtable to L1.
  STALL_MEMTABLE_COMPACTION_MICROS,
  // DEPRECATED write throttle because of too many files in L0
  STALL_L0_NUM_FILES_MICROS,
  // Writer has to wait for compaction or flush to finish.
  STALL_MICROS,
  // The wait time for db mutex.
  DB_MUTEX_WAIT_MICROS,
  RATE_LIMIT_DELAY_MILLIS,
  NO_ITERATORS,  // number of iterators currently open

  // Number of MultiGet calls, keys read, and bytes read
  NUMBER_MULTIGET_CALLS,
  NUMBER_MULTIGET_KEYS_READ,
  NUMBER_MULTIGET_BYTES_READ,

  // Number of deletes records that were not required to be
  // written to storage because key does not exist
  NUMBER_FILTERED_DELETES,
  NUMBER_MERGE_FAILURES,
  SEQUENCE_NUMBER,

  // number of times bloom was checked before creating iterator on a
  // file, and the number of times the check was useful in avoiding
  // iterator creation (and thus likely IOPs).
  BLOOM_FILTER_PREFIX_CHECKED,
  BLOOM_FILTER_PREFIX_USEFUL,

  // Number of times we had to reseek inside an iteration to skip
  // over large number of keys with same userkey.
  NUMBER_OF_RESEEKS_IN_ITERATION,

  // Record the number of calls to GetUpadtesSince. Useful to keep track of
  // transaction log iterator refreshes
  GET_UPDATES_SINCE_CALLS,
  BLOCK_CACHE_COMPRESSED_MISS,  // miss in the compressed block cache
  BLOCK_CACHE_COMPRESSED_HIT,   // hit in the compressed block cache
  WAL_FILE_SYNCED,              // Number of times WAL sync is done
  WAL_FILE_BYTES,               // Number of bytes written to WAL

  // Writes can be processed by requesting thread or by the thread at the
  // head of the writers queue.
  WRITE_DONE_BY_SELF,
  WRITE_DONE_BY_OTHER,
  WRITE_TIMEDOUT,       // Number of writes ending up with timed-out.
  WRITE_WITH_WAL,       // Number of Write calls that request WAL
  COMPACT_READ_BYTES,   // Bytes read during compaction
  COMPACT_WRITE_BYTES,  // Bytes written during compaction
  FLUSH_WRITE_BYTES,    // Bytes written during flush

  // Number of table's properties loaded directly from file, without creating
  // table reader object.
  NUMBER_DIRECT_LOAD_TABLE_PROPERTIES,
  NUMBER_SUPERVERSION_ACQUIRES,
  NUMBER_SUPERVERSION_RELEASES,
  NUMBER_SUPERVERSION_CLEANUPS,
  NUMBER_BLOCK_NOT_COMPRESSED,
  MERGE_OPERATION_TOTAL_TIME,
  FILTER_OPERATION_TOTAL_TIME,

  // Row cache.
  ROW_CACHE_HIT,
  ROW_CACHE_MISS,

  TICKER_ENUM_MAX
};

// The order of items listed in  Tickers should be the same as
// the order listed in TickersNameMap
const std::vector<std::pair<Tickers, std::string>> TickersNameMap = {
    {BLOCK_CACHE_MISS, "rocksdb.block.cache.miss"},
    {BLOCK_CACHE_HIT, "rocksdb.block.cache.hit"},
    {BLOCK_CACHE_ADD, "rocksdb.block.cache.add"},
    {BLOCK_CACHE_INDEX_MISS, "rocksdb.block.cache.index.miss"},
    {BLOCK_CACHE_INDEX_HIT, "rocksdb.block.cache.index.hit"},
    {BLOCK_CACHE_FILTER_MISS, "rocksdb.block.cache.filter.miss"},
    {BLOCK_CACHE_FILTER_HIT, "rocksdb.block.cache.filter.hit"},
    {BLOCK_CACHE_DATA_MISS, "rocksdb.block.cache.data.miss"},
    {BLOCK_CACHE_DATA_HIT, "rocksdb.block.cache.data.hit"},
    {BLOCK_CACHE_BYTES_READ, "rocksdb.block.cache.bytes.read"},
    {BLOCK_CACHE_BYTES_WRITE, "rocksdb.block.cache.bytes.write"},
    {BLOOM_FILTER_USEFUL, "rocksdb.bloom.filter.useful"},
    {MEMTABLE_HIT, "rocksdb.memtable.hit"},
    {MEMTABLE_MISS, "rocksdb.memtable.miss"},
    {GET_HIT_L0, "rocksdb.l0.hit"},
    {GET_HIT_L1, "rocksdb.l1.hit"},
    {GET_HIT_L2_AND_UP, "rocksdb.l2andup.hit"},
    {COMPACTION_KEY_DROP_NEWER_ENTRY, "rocksdb.compaction.key.drop.new"},
    {COMPACTION_KEY_DROP_OBSOLETE, "rocksdb.compaction.key.drop.obsolete"},
    {COMPACTION_KEY_DROP_USER, "rocksdb.compaction.key.drop.user"},
    {NUMBER_KEYS_WRITTEN, "rocksdb.number.keys.written"},
    {NUMBER_KEYS_READ, "rocksdb.number.keys.read"},
    {NUMBER_KEYS_UPDATED, "rocksdb.number.keys.updated"},
    {BYTES_WRITTEN, "rocksdb.bytes.written"},
    {BYTES_READ, "rocksdb.bytes.read"},
    {NUMBER_DB_SEEK, "rocksdb.number.db.seek"},
    {NUMBER_DB_NEXT, "rocksdb.number.db.next"},
    {NUMBER_DB_PREV, "rocksdb.number.db.prev"},
    {NUMBER_DB_SEEK_FOUND, "rocksdb.number.db.seek.found"},
    {NUMBER_DB_NEXT_FOUND, "rocksdb.number.db.next.found"},
    {NUMBER_DB_PREV_FOUND, "rocksdb.number.db.prev.found"},
    {ITER_BYTES_READ, "rocksdb.db.iter.bytes.read"},
    {NO_FILE_CLOSES, "rocksdb.no.file.closes"},
    {NO_FILE_OPENS, "rocksdb.no.file.opens"},
    {NO_FILE_ERRORS, "rocksdb.no.file.errors"},
    {STALL_L0_SLOWDOWN_MICROS, "rocksdb.l0.slowdown.micros"},
    {STALL_MEMTABLE_COMPACTION_MICROS, "rocksdb.memtable.compaction.micros"},
    {STALL_L0_NUM_FILES_MICROS, "rocksdb.l0.num.files.stall.micros"},
    {STALL_MICROS, "rocksdb.stall.micros"},
    {DB_MUTEX_WAIT_MICROS, "rocksdb.db.mutex.wait.micros"},
    {RATE_LIMIT_DELAY_MILLIS, "rocksdb.rate.limit.delay.millis"},
    {NO_ITERATORS, "rocksdb.num.iterators"},
    {NUMBER_MULTIGET_CALLS, "rocksdb.number.multiget.get"},
    {NUMBER_MULTIGET_KEYS_READ, "rocksdb.number.multiget.keys.read"},
    {NUMBER_MULTIGET_BYTES_READ, "rocksdb.number.multiget.bytes.read"},
    {NUMBER_FILTERED_DELETES, "rocksdb.number.deletes.filtered"},
    {NUMBER_MERGE_FAILURES, "rocksdb.number.merge.failures"},
    {SEQUENCE_NUMBER, "rocksdb.sequence.number"},
    {BLOOM_FILTER_PREFIX_CHECKED, "rocksdb.bloom.filter.prefix.checked"},
    {BLOOM_FILTER_PREFIX_USEFUL, "rocksdb.bloom.filter.prefix.useful"},
    {NUMBER_OF_RESEEKS_IN_ITERATION, "rocksdb.number.reseeks.iteration"},
    {GET_UPDATES_SINCE_CALLS, "rocksdb.getupdatessince.calls"},
    {BLOCK_CACHE_COMPRESSED_MISS, "rocksdb.block.cachecompressed.miss"},
    {BLOCK_CACHE_COMPRESSED_HIT, "rocksdb.block.cachecompressed.hit"},
    {WAL_FILE_SYNCED, "rocksdb.wal.synced"},
    {WAL_FILE_BYTES, "rocksdb.wal.bytes"},
    {WRITE_DONE_BY_SELF, "rocksdb.write.self"},
    {WRITE_DONE_BY_OTHER, "rocksdb.write.other"},
    {WRITE_WITH_WAL, "rocksdb.write.wal"},
    {FLUSH_WRITE_BYTES, "rocksdb.flush.write.bytes"},
    {COMPACT_READ_BYTES, "rocksdb.compact.read.bytes"},
    {COMPACT_WRITE_BYTES, "rocksdb.compact.write.bytes"},
    {NUMBER_DIRECT_LOAD_TABLE_PROPERTIES,
     "rocksdb.number.direct.load.table.properties"},
    {NUMBER_SUPERVERSION_ACQUIRES, "rocksdb.number.superversion_acquires"},
    {NUMBER_SUPERVERSION_RELEASES, "rocksdb.number.superversion_releases"},
    {NUMBER_SUPERVERSION_CLEANUPS, "rocksdb.number.superversion_cleanups"},
    {NUMBER_BLOCK_NOT_COMPRESSED, "rocksdb.number.block.not_compressed"},
    {MERGE_OPERATION_TOTAL_TIME, "rocksdb.merge.operation.time.nanos"},
    {FILTER_OPERATION_TOTAL_TIME, "rocksdb.filter.operation.time.nanos"},
    {ROW_CACHE_HIT, "rocksdb.row.cache.hit"},
    {ROW_CACHE_MISS, "rocksdb.row.cache.miss"},
};

/**
 * Keep adding histogram's here.
 * Any histogram whould have value less than HISTOGRAM_ENUM_MAX
 * Add a new Histogram by assigning it the current value of HISTOGRAM_ENUM_MAX
 * Add a string representation in HistogramsNameMap below
 * And increment HISTOGRAM_ENUM_MAX
 */
enum Histograms : uint32_t {
  DB_GET = 0,
  DB_WRITE,
  COMPACTION_TIME,
  SUBCOMPACTION_SETUP_TIME,
  TABLE_SYNC_MICROS,
  COMPACTION_OUTFILE_SYNC_MICROS,
  WAL_FILE_SYNC_MICROS,
  MANIFEST_FILE_SYNC_MICROS,
  // TIME SPENT IN IO DURING TABLE OPEN
  TABLE_OPEN_IO_MICROS,
  DB_MULTIGET,
  READ_BLOCK_COMPACTION_MICROS,
  READ_BLOCK_GET_MICROS,
  WRITE_RAW_BLOCK_MICROS,
  STALL_L0_SLOWDOWN_COUNT,
  STALL_MEMTABLE_COMPACTION_COUNT,
  STALL_L0_NUM_FILES_COUNT,
  HARD_RATE_LIMIT_DELAY_COUNT,
  SOFT_RATE_LIMIT_DELAY_COUNT,
  NUM_FILES_IN_SINGLE_COMPACTION,
  DB_SEEK,
  WRITE_STALL,
  SST_READ_MICROS,
  // The number of subcompactions actually scheduled during a compaction
  NUM_SUBCOMPACTIONS_SCHEDULED,
  HISTOGRAM_ENUM_MAX,  // TODO(ldemailly): enforce HistogramsNameMap match
};

const std::vector<std::pair<Histograms, std::string>> HistogramsNameMap = {
    {DB_GET, "rocksdb.db.get.micros"},
    {DB_WRITE, "rocksdb.db.write.micros"},
    {COMPACTION_TIME, "rocksdb.compaction.times.micros"},
    {SUBCOMPACTION_SETUP_TIME, "rocksdb.subcompaction.setup.times.micros"},
    {TABLE_SYNC_MICROS, "rocksdb.table.sync.micros"},
    {COMPACTION_OUTFILE_SYNC_MICROS, "rocksdb.compaction.outfile.sync.micros"},
    {WAL_FILE_SYNC_MICROS, "rocksdb.wal.file.sync.micros"},
    {MANIFEST_FILE_SYNC_MICROS, "rocksdb.manifest.file.sync.micros"},
    {TABLE_OPEN_IO_MICROS, "rocksdb.table.open.io.micros"},
    {DB_MULTIGET, "rocksdb.db.multiget.micros"},
    {READ_BLOCK_COMPACTION_MICROS, "rocksdb.read.block.compaction.micros"},
    {READ_BLOCK_GET_MICROS, "rocksdb.read.block.get.micros"},
    {WRITE_RAW_BLOCK_MICROS, "rocksdb.write.raw.block.micros"},
    {STALL_L0_SLOWDOWN_COUNT, "rocksdb.l0.slowdown.count"},
    {STALL_MEMTABLE_COMPACTION_COUNT, "rocksdb.memtable.compaction.count"},
    {STALL_L0_NUM_FILES_COUNT, "rocksdb.num.files.stall.count"},
    {HARD_RATE_LIMIT_DELAY_COUNT, "rocksdb.hard.rate.limit.delay.count"},
    {SOFT_RATE_LIMIT_DELAY_COUNT, "rocksdb.soft.rate.limit.delay.count"},
    {NUM_FILES_IN_SINGLE_COMPACTION, "rocksdb.numfiles.in.singlecompaction"},
    {DB_SEEK, "rocksdb.db.seek.micros"},
    {WRITE_STALL, "rocksdb.db.write.stall"},
    {SST_READ_MICROS, "rocksdb.sst.read.micros"},
    {NUM_SUBCOMPACTIONS_SCHEDULED, "rocksdb.num.subcompactions.scheduled"},
};

struct HistogramData {
  double median;
  double percentile95;
  double percentile99;
  double average;
  double standard_deviation;
};

// Analyze the performance of a db
class Statistics {
 public:
  virtual ~Statistics() {}

  virtual uint64_t getTickerCount(uint32_t tickerType) const = 0;
  virtual void histogramData(uint32_t type,
                             HistogramData* const data) const = 0;
  virtual std::string getHistogramString(uint32_t type) const { return ""; }
  virtual void recordTick(uint32_t tickerType, uint64_t count = 0) = 0;
  virtual void setTickerCount(uint32_t tickerType, uint64_t count) = 0;
  virtual void measureTime(uint32_t histogramType, uint64_t time) = 0;

  // String representation of the statistic object.
  virtual std::string ToString() const {
    // Do nothing by default
    return std::string("ToString(): not implemented");
  }

  // Override this function to disable particular histogram collection
  virtual bool HistEnabledForType(uint32_t type) const {
    return type < HISTOGRAM_ENUM_MAX;
  }
};

// Create a concrete DBStatistics object
std::shared_ptr<Statistics> CreateDBStatistics();

}  // namespace rocksdb

#endif  // STORAGE_ROCKSDB_INCLUDE_STATISTICS_H_
#line 1 "/home/evan/source/rocksdb/include/rocksdb/compaction_filter.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
// Copyright (c) 2013 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_ROCKSDB_INCLUDE_COMPACTION_FILTER_H_
#define STORAGE_ROCKSDB_INCLUDE_COMPACTION_FILTER_H_

#include <memory>
#include <string>
#include <vector>

namespace rocksdb {

class Slice;
class SliceTransform;

// Context information of a compaction run
struct CompactionFilterContext {
  // Does this compaction run include all data files
  bool is_full_compaction;
  // Is this compaction requested by the client (true),
  // or is it occurring as an automatic compaction process
  bool is_manual_compaction;
};

// CompactionFilter allows an application to modify/delete a key-value at
// the time of compaction.

class CompactionFilter {
 public:
  // Context information of a compaction run
  struct Context {
    // Does this compaction run include all data files
    bool is_full_compaction;
    // Is this compaction requested by the client (true),
    // or is it occurring as an automatic compaction process
    bool is_manual_compaction;
  };

  virtual ~CompactionFilter() {}

  // The compaction process invokes this
  // method for kv that is being compacted. A return value
  // of false indicates that the kv should be preserved in the
  // output of this compaction run and a return value of true
  // indicates that this key-value should be removed from the
  // output of the compaction.  The application can inspect
  // the existing value of the key and make decision based on it.
  //
  // Key-Values that are results of merge operation during compaction are not
  // passed into this function. Currently, when you have a mix of Put()s and
  // Merge()s on a same key, we only guarantee to process the merge operands
  // through the compaction filters. Put()s might be processed, or might not.
  //
  // When the value is to be preserved, the application has the option
  // to modify the existing_value and pass it back through new_value.
  // value_changed needs to be set to true in this case.
  //
  // If you use snapshot feature of RocksDB (i.e. call GetSnapshot() API on a
  // DB* object), CompactionFilter might not be very useful for you. Due to
  // guarantees we need to maintain, compaction process will not call Filter()
  // on any keys that were written before the latest snapshot. In other words,
  // compaction will only call Filter() on keys written after your most recent
  // call to GetSnapshot(). In most cases, Filter() will not be called very
  // often. This is something we're fixing. See the discussion at:
  // https://www.facebook.com/groups/mysqlonrocksdb/permalink/999723240091865/
  //
  // If multithreaded compaction is being used *and* a single CompactionFilter
  // instance was supplied via Options::compaction_filter, this method may be
  // called from different threads concurrently.  The application must ensure
  // that the call is thread-safe.
  //
  // If the CompactionFilter was created by a factory, then it will only ever
  // be used by a single thread that is doing the compaction run, and this
  // call does not need to be thread-safe.  However, multiple filters may be
  // in existence and operating concurrently.
  //
  // The last paragraph is not true if you set max_subcompactions to more than
  // 1. In that case, subcompaction from multiple threads may call a single
  // CompactionFilter concurrently.
  virtual bool Filter(int level,
                      const Slice& key,
                      const Slice& existing_value,
                      std::string* new_value,
                      bool* value_changed) const = 0;

  // The compaction process invokes this method on every merge operand. If this
  // method returns true, the merge operand will be ignored and not written out
  // in the compaction output
  virtual bool FilterMergeOperand(int level, const Slice& key,
                                  const Slice& operand) const {
    return false;
  }

  // Returns a name that identifies this compaction filter.
  // The name will be printed to LOG file on start up for diagnosis.
  virtual const char* Name() const = 0;
};

// Each compaction will create a new CompactionFilter allowing the
// application to know about different compactions
class CompactionFilterFactory {
 public:
  virtual ~CompactionFilterFactory() { }

  virtual std::unique_ptr<CompactionFilter> CreateCompactionFilter(
      const CompactionFilter::Context& context) = 0;

  // Returns a name that identifies this compaction filter factory.
  virtual const char* Name() const = 0;
};

// Default implementation of CompactionFilterFactory which does not
// return any filter
class DefaultCompactionFilterFactory : public CompactionFilterFactory {
 public:
  virtual std::unique_ptr<CompactionFilter> CreateCompactionFilter(
      const CompactionFilter::Context& context) override {
    return std::unique_ptr<CompactionFilter>(nullptr);
  }

  virtual const char* Name() const override {
    return "DefaultCompactionFilterFactory";
  }
};

}  // namespace rocksdb

#endif  // STORAGE_ROCKSDB_INCLUDE_COMPACTION_FILTER_H_
#line 1 "/home/evan/source/rocksdb/include/rocksdb/flush_block_policy.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.


#include <string>

namespace rocksdb {

class Slice;
class BlockBuilder;
struct Options;

// FlushBlockPolicy provides a configurable way to determine when to flush a
// block in the block based tables,
class FlushBlockPolicy {
 public:
  // Keep track of the key/value sequences and return the boolean value to
  // determine if table builder should flush current data block.
  virtual bool Update(const Slice& key,
                      const Slice& value) = 0;

  virtual ~FlushBlockPolicy() { }
};

class FlushBlockPolicyFactory {
 public:
  // Return the name of the flush block policy.
  virtual const char* Name() const = 0;

  // Return a new block flush policy that flushes data blocks by data size.
  // FlushBlockPolicy may need to access the metadata of the data block
  // builder to determine when to flush the blocks.
  //
  // Callers must delete the result after any database that is using the
  // result has been closed.
  virtual FlushBlockPolicy* NewFlushBlockPolicy(
      const BlockBasedTableOptions& table_options,
      const BlockBuilder& data_block_builder) const = 0;

  virtual ~FlushBlockPolicyFactory() { }
};

class FlushBlockBySizePolicyFactory : public FlushBlockPolicyFactory {
 public:
  FlushBlockBySizePolicyFactory() {}

  virtual const char* Name() const override {
    return "FlushBlockBySizePolicyFactory";
  }

  virtual FlushBlockPolicy* NewFlushBlockPolicy(
      const BlockBasedTableOptions& table_options,
      const BlockBuilder& data_block_builder) const override;
};

}  // rocksdb
#line 1 "/home/evan/source/rocksdb/include/rocksdb/iostats_context.h"
// Copyright (c) 2014, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <stdint.h>
#include <string>

#line 1 "/home/evan/source/rocksdb/include/rocksdb/perf_level.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#ifndef INCLUDE_ROCKSDB_PERF_LEVEL_H_
#define INCLUDE_ROCKSDB_PERF_LEVEL_H_

#include <stdint.h>
#include <string>

namespace rocksdb {

// How much perf stats to collect. Affects perf_context and iostats_context.

enum PerfLevel {
  kDisable        = 0,  // disable perf stats
  kEnableCount    = 1,  // enable only count stats
  kEnableTime     = 2   // enable time stats too
};

// set the perf stats level for current thread
void SetPerfLevel(PerfLevel level);

// get current perf stats level for current thread
PerfLevel GetPerfLevel();

}  // namespace rocksdb

#endif  // INCLUDE_ROCKSDB_PERF_LEVEL_H_
#line 10 "/home/evan/source/rocksdb/include/rocksdb/iostats_context.h"

// A thread local context for gathering io-stats efficiently and transparently.
// Use SetPerfLevel(PerfLevel::kEnableTime) to enable time stats.

namespace rocksdb {

struct IOStatsContext {
  // reset all io-stats counter to zero
  void Reset();

  std::string ToString() const;

  // the thread pool id
  uint64_t thread_pool_id;

  // number of bytes that has been written.
  uint64_t bytes_written;
  // number of bytes that has been read.
  uint64_t bytes_read;

  // time spent in open() and fopen().
  uint64_t open_nanos;
  // time spent in fallocate().
  uint64_t allocate_nanos;
  // time spent in write() and pwrite().
  uint64_t write_nanos;
  // time spent in read() and pread()
  uint64_t read_nanos;
  // time spent in sync_file_range().
  uint64_t range_sync_nanos;
  // time spent in fsync
  uint64_t fsync_nanos;
  // time spent in preparing write (fallocate etc).
  uint64_t prepare_write_nanos;
  // time spent in Logger::Logv().
  uint64_t logger_nanos;
};

#ifndef IOS_CROSS_COMPILE
# ifdef _WIN32
extern __declspec(thread) IOStatsContext iostats_context;
# else
extern __thread IOStatsContext iostats_context;
# endif
#endif  // IOS_CROSS_COMPILE

}  // namespace rocksdb
#line 1 "/home/evan/source/rocksdb/include/rocksdb/convenience.h"
// Copyright (c) 2014, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.


#include <unordered_map>
#include <string>

namespace rocksdb {

#ifndef ROCKSDB_LITE
// Take a map of option name and option value, apply them into the
// base_options, and return the new options as a result.
//
// If input_strings_escaped is set to true, then each escaped characters
// prefixed by '\' in the the values of the opts_map will be further
// converted back to the raw string before assigning to the associated
// options.
Status GetColumnFamilyOptionsFromMap(
    const ColumnFamilyOptions& base_options,
    const std::unordered_map<std::string, std::string>& opts_map,
    ColumnFamilyOptions* new_options, bool input_strings_escaped = false);

// Take a map of option name and option value, apply them into the
// base_options, and return the new options as a result.
//
// If input_strings_escaped is set to true, then each escaped characters
// prefixed by '\' in the the values of the opts_map will be further
// converted back to the raw string before assigning to the associated
// options.
Status GetDBOptionsFromMap(
    const DBOptions& base_options,
    const std::unordered_map<std::string, std::string>& opts_map,
    DBOptions* new_options, bool input_strings_escaped = false);

Status GetBlockBasedTableOptionsFromMap(
    const BlockBasedTableOptions& table_options,
    const std::unordered_map<std::string, std::string>& opts_map,
    BlockBasedTableOptions* new_table_options);

// Take a string representation of option names and  values, apply them into the
// base_options, and return the new options as a result. The string has the
// following format:
//   "write_buffer_size=1024;max_write_buffer_number=2"
// Nested options config is also possible. For example, you can define
// BlockBasedTableOptions as part of the string for block-based table factory:
//   "write_buffer_size=1024;block_based_table_factory={block_size=4k};"
//   "max_write_buffer_num=2"
Status GetColumnFamilyOptionsFromString(
    const ColumnFamilyOptions& base_options,
    const std::string& opts_str,
    ColumnFamilyOptions* new_options);

Status GetDBOptionsFromString(
    const DBOptions& base_options,
    const std::string& opts_str,
    DBOptions* new_options);

Status GetStringFromDBOptions(std::string* opts_str,
                              const DBOptions& db_options,
                              const std::string& delimiter = ";  ");

Status GetStringFromColumnFamilyOptions(std::string* opts_str,
                                        const ColumnFamilyOptions& db_options,
                                        const std::string& delimiter = ";  ");

Status GetBlockBasedTableOptionsFromString(
    const BlockBasedTableOptions& table_options,
    const std::string& opts_str,
    BlockBasedTableOptions* new_table_options);

Status GetOptionsFromString(const Options& base_options,
                            const std::string& opts_str, Options* new_options);

/// Request stopping background work, if wait is true wait until it's done
void CancelAllBackgroundWork(DB* db, bool wait = false);
#endif  // ROCKSDB_LITE

}  // namespace rocksdb
#line 1 "/home/evan/source/rocksdb/include/rocksdb/merge_operator.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#ifndef STORAGE_ROCKSDB_INCLUDE_MERGE_OPERATOR_H_
#define STORAGE_ROCKSDB_INCLUDE_MERGE_OPERATOR_H_

#include <deque>
#include <memory>
#include <string>


namespace rocksdb {

class Slice;
class Logger;

// The Merge Operator
//
// Essentially, a MergeOperator specifies the SEMANTICS of a merge, which only
// client knows. It could be numeric addition, list append, string
// concatenation, edit data structure, ... , anything.
// The library, on the other hand, is concerned with the exercise of this
// interface, at the right time (during get, iteration, compaction...)
//
// To use merge, the client needs to provide an object implementing one of
// the following interfaces:
//  a) AssociativeMergeOperator - for most simple semantics (always take
//    two values, and merge them into one value, which is then put back
//    into rocksdb); numeric addition and string concatenation are examples;
//
//  b) MergeOperator - the generic class for all the more abstract / complex
//    operations; one method (FullMerge) to merge a Put/Delete value with a
//    merge operand; and another method (PartialMerge) that merges multiple
//    operands together. this is especially useful if your key values have
//    complex structures but you would still like to support client-specific
//    incremental updates.
//
// AssociativeMergeOperator is simpler to implement. MergeOperator is simply
// more powerful.
//
// Refer to rocksdb-merge wiki for more details and example implementations.
//
class MergeOperator {
 public:
  virtual ~MergeOperator() {}

  // Gives the client a way to express the read -> modify -> write semantics
  // key:      (IN)    The key that's associated with this merge operation.
  //                   Client could multiplex the merge operator based on it
  //                   if the key space is partitioned and different subspaces
  //                   refer to different types of data which have different
  //                   merge operation semantics
  // existing: (IN)    null indicates that the key does not exist before this op
  // operand_list:(IN) the sequence of merge operations to apply, front() first.
  // new_value:(OUT)   Client is responsible for filling the merge result here.
  // The string that new_value is pointing to will be empty.
  // logger:   (IN)    Client could use this to log errors during merge.
  //
  // Return true on success.
  // All values passed in will be client-specific values. So if this method
  // returns false, it is because client specified bad data or there was
  // internal corruption. This will be treated as an error by the library.
  //
  // Also make use of the *logger for error messages.
  virtual bool FullMerge(const Slice& key,
                         const Slice* existing_value,
                         const std::deque<std::string>& operand_list,
                         std::string* new_value,
                         Logger* logger) const = 0;

  // This function performs merge(left_op, right_op)
  // when both the operands are themselves merge operation types
  // that you would have passed to a DB::Merge() call in the same order
  // (i.e.: DB::Merge(key,left_op), followed by DB::Merge(key,right_op)).
  //
  // PartialMerge should combine them into a single merge operation that is
  // saved into *new_value, and then it should return true.
  // *new_value should be constructed such that a call to
  // DB::Merge(key, *new_value) would yield the same result as a call
  // to DB::Merge(key, left_op) followed by DB::Merge(key, right_op).
  //
  // The string that new_value is pointing to will be empty.
  //
  // The default implementation of PartialMergeMulti will use this function
  // as a helper, for backward compatibility.  Any successor class of
  // MergeOperator should either implement PartialMerge or PartialMergeMulti,
  // although implementing PartialMergeMulti is suggested as it is in general
  // more effective to merge multiple operands at a time instead of two
  // operands at a time.
  //
  // If it is impossible or infeasible to combine the two operations,
  // leave new_value unchanged and return false. The library will
  // internally keep track of the operations, and apply them in the
  // correct order once a base-value (a Put/Delete/End-of-Database) is seen.
  //
  // TODO: Presently there is no way to differentiate between error/corruption
  // and simply "return false". For now, the client should simply return
  // false in any case it cannot perform partial-merge, regardless of reason.
  // If there is corruption in the data, handle it in the FullMerge() function,
  // and return false there.  The default implementation of PartialMerge will
  // always return false.
  virtual bool PartialMerge(const Slice& key, const Slice& left_operand,
                            const Slice& right_operand, std::string* new_value,
                            Logger* logger) const {
    return false;
  }

  // This function performs merge when all the operands are themselves merge
  // operation types that you would have passed to a DB::Merge() call in the
  // same order (front() first)
  // (i.e. DB::Merge(key, operand_list[0]), followed by
  //  DB::Merge(key, operand_list[1]), ...)
  //
  // PartialMergeMulti should combine them into a single merge operation that is
  // saved into *new_value, and then it should return true.  *new_value should
  // be constructed such that a call to DB::Merge(key, *new_value) would yield
  // the same result as subquential individual calls to DB::Merge(key, operand)
  // for each operand in operand_list from front() to back().
  //
  // The string that new_value is pointing to will be empty.
  //
  // The PartialMergeMulti function will be called only when the list of
  // operands are long enough. The minimum amount of operands that will be
  // passed to the function are specified by the "min_partial_merge_operands"
  // option.
  //
  // In the default implementation, PartialMergeMulti will invoke PartialMerge
  // multiple times, where each time it only merges two operands.  Developers
  // should either implement PartialMergeMulti, or implement PartialMerge which
  // is served as the helper function of the default PartialMergeMulti.
  virtual bool PartialMergeMulti(const Slice& key,
                                 const std::deque<Slice>& operand_list,
                                 std::string* new_value, Logger* logger) const;

  // The name of the MergeOperator. Used to check for MergeOperator
  // mismatches (i.e., a DB created with one MergeOperator is
  // accessed using a different MergeOperator)
  // TODO: the name is currently not stored persistently and thus
  //       no checking is enforced. Client is responsible for providing
  //       consistent MergeOperator between DB opens.
  virtual const char* Name() const = 0;
};

// The simpler, associative merge operator.
class AssociativeMergeOperator : public MergeOperator {
 public:
  virtual ~AssociativeMergeOperator() {}

  // Gives the client a way to express the read -> modify -> write semantics
  // key:           (IN) The key that's associated with this merge operation.
  // existing_value:(IN) null indicates the key does not exist before this op
  // value:         (IN) the value to update/merge the existing_value with
  // new_value:    (OUT) Client is responsible for filling the merge result
  // here. The string that new_value is pointing to will be empty.
  // logger:        (IN) Client could use this to log errors during merge.
  //
  // Return true on success.
  // All values passed in will be client-specific values. So if this method
  // returns false, it is because client specified bad data or there was
  // internal corruption. The client should assume that this will be treated
  // as an error by the library.
  virtual bool Merge(const Slice& key,
                     const Slice* existing_value,
                     const Slice& value,
                     std::string* new_value,
                     Logger* logger) const = 0;


 private:
  // Default implementations of the MergeOperator functions
  virtual bool FullMerge(const Slice& key,
                         const Slice* existing_value,
                         const std::deque<std::string>& operand_list,
                         std::string* new_value,
                         Logger* logger) const override;

  virtual bool PartialMerge(const Slice& key,
                            const Slice& left_operand,
                            const Slice& right_operand,
                            std::string* new_value,
                            Logger* logger) const override;
};

}  // namespace rocksdb

#endif  // STORAGE_ROCKSDB_INCLUDE_MERGE_OPERATOR_H_
#line 1 "/home/evan/source/rocksdb/include/rocksdb/utilities/backupable_db.h"
//  Copyright (c) 2013, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.
//
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef ROCKSDB_LITE

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>
#include <string>
#include <map>
#include <vector>
#include <functional>

#line 1 "/home/evan/source/rocksdb/include/rocksdb/utilities/stackable_db.h"
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include <string>

#ifdef _WIN32
// Windows API macro interference
#undef DeleteFile
#endif


namespace rocksdb {

// This class contains APIs to stack rocksdb wrappers.Eg. Stack TTL over base d
class StackableDB : public DB {
 public:
  // StackableDB is the owner of db now!
  explicit StackableDB(DB* db) : db_(db) {}

  ~StackableDB() {
    delete db_;
  }

  virtual DB* GetBaseDB() {
    return db_;
  }

  virtual DB* GetRootDB() override { return db_->GetRootDB(); }

  virtual Status CreateColumnFamily(const ColumnFamilyOptions& options,
                                    const std::string& column_family_name,
                                    ColumnFamilyHandle** handle) override {
    return db_->CreateColumnFamily(options, column_family_name, handle);
  }

  virtual Status DropColumnFamily(ColumnFamilyHandle* column_family) override {
    return db_->DropColumnFamily(column_family);
  }

  using DB::Put;
  virtual Status Put(const WriteOptions& options,
                     ColumnFamilyHandle* column_family, const Slice& key,
                     const Slice& val) override {
    return db_->Put(options, column_family, key, val);
  }

  using DB::Get;
  virtual Status Get(const ReadOptions& options,
                     ColumnFamilyHandle* column_family, const Slice& key,
                     std::string* value) override {
    return db_->Get(options, column_family, key, value);
  }

  using DB::MultiGet;
  virtual std::vector<Status> MultiGet(
      const ReadOptions& options,
      const std::vector<ColumnFamilyHandle*>& column_family,
      const std::vector<Slice>& keys,
      std::vector<std::string>* values) override {
    return db_->MultiGet(options, column_family, keys, values);
  }

  using DB::AddFile;
  virtual Status AddFile(ColumnFamilyHandle* column_family,
                         const ExternalSstFileInfo* file_info,
                         bool move_file) override {
    return db_->AddFile(column_family, file_info, move_file);
  }
  virtual Status AddFile(ColumnFamilyHandle* column_family,
                         const std::string& file_path,
                         bool move_file) override {
    return db_->AddFile(column_family, file_path, move_file);
  }

  using DB::KeyMayExist;
  virtual bool KeyMayExist(const ReadOptions& options,
                           ColumnFamilyHandle* column_family, const Slice& key,
                           std::string* value,
                           bool* value_found = nullptr) override {
    return db_->KeyMayExist(options, column_family, key, value, value_found);
  }

  using DB::Delete;
  virtual Status Delete(const WriteOptions& wopts,
                        ColumnFamilyHandle* column_family,
                        const Slice& key) override {
    return db_->Delete(wopts, column_family, key);
  }

  using DB::SingleDelete;
  virtual Status SingleDelete(const WriteOptions& wopts,
                              ColumnFamilyHandle* column_family,
                              const Slice& key) override {
    return db_->SingleDelete(wopts, column_family, key);
  }

  using DB::Merge;
  virtual Status Merge(const WriteOptions& options,
                       ColumnFamilyHandle* column_family, const Slice& key,
                       const Slice& value) override {
    return db_->Merge(options, column_family, key, value);
  }


  virtual Status Write(const WriteOptions& opts, WriteBatch* updates)
    override {
      return db_->Write(opts, updates);
  }

  using DB::NewIterator;
  virtual Iterator* NewIterator(const ReadOptions& opts,
                                ColumnFamilyHandle* column_family) override {
    return db_->NewIterator(opts, column_family);
  }

  virtual Status NewIterators(
      const ReadOptions& options,
      const std::vector<ColumnFamilyHandle*>& column_families,
      std::vector<Iterator*>* iterators) override {
    return db_->NewIterators(options, column_families, iterators);
  }


  virtual const Snapshot* GetSnapshot() override {
    return db_->GetSnapshot();
  }

  virtual void ReleaseSnapshot(const Snapshot* snapshot) override {
    return db_->ReleaseSnapshot(snapshot);
  }

  using DB::GetProperty;
  virtual bool GetProperty(ColumnFamilyHandle* column_family,
                           const Slice& property, std::string* value) override {
    return db_->GetProperty(column_family, property, value);
  }

  using DB::GetIntProperty;
  virtual bool GetIntProperty(ColumnFamilyHandle* column_family,
                              const Slice& property, uint64_t* value) override {
    return db_->GetIntProperty(column_family, property, value);
  }

  using DB::GetApproximateSizes;
  virtual void GetApproximateSizes(ColumnFamilyHandle* column_family,
                                   const Range* r, int n, uint64_t* sizes,
                                   bool include_memtable = false) override {
      return db_->GetApproximateSizes(column_family, r, n, sizes);
  }

  using DB::CompactRange;
  virtual Status CompactRange(const CompactRangeOptions& options,
                              ColumnFamilyHandle* column_family,
                              const Slice* begin, const Slice* end) override {
    return db_->CompactRange(options, column_family, begin, end);
  }

  using DB::CompactFiles;
  virtual Status CompactFiles(
      const CompactionOptions& compact_options,
      ColumnFamilyHandle* column_family,
      const std::vector<std::string>& input_file_names,
      const int output_level, const int output_path_id = -1) override {
    return db_->CompactFiles(
        compact_options, column_family, input_file_names,
        output_level, output_path_id);
  }

  virtual Status PauseBackgroundWork() override {
    return db_->PauseBackgroundWork();
  }
  virtual Status ContinueBackgroundWork() override {
    return db_->ContinueBackgroundWork();
  }

  using DB::NumberLevels;
  virtual int NumberLevels(ColumnFamilyHandle* column_family) override {
    return db_->NumberLevels(column_family);
  }

  using DB::MaxMemCompactionLevel;
  virtual int MaxMemCompactionLevel(ColumnFamilyHandle* column_family)
      override {
    return db_->MaxMemCompactionLevel(column_family);
  }

  using DB::Level0StopWriteTrigger;
  virtual int Level0StopWriteTrigger(ColumnFamilyHandle* column_family)
      override {
    return db_->Level0StopWriteTrigger(column_family);
  }

  virtual const std::string& GetName() const override {
    return db_->GetName();
  }

  virtual Env* GetEnv() const override {
    return db_->GetEnv();
  }

  using DB::GetOptions;
  virtual const Options& GetOptions(ColumnFamilyHandle* column_family) const
      override {
    return db_->GetOptions(column_family);
  }

  using DB::GetDBOptions;
  virtual const DBOptions& GetDBOptions() const override {
    return db_->GetDBOptions();
  }

  using DB::Flush;
  virtual Status Flush(const FlushOptions& fopts,
                       ColumnFamilyHandle* column_family) override {
    return db_->Flush(fopts, column_family);
  }

  virtual Status SyncWAL() override {
    return db_->SyncWAL();
  }

#ifndef ROCKSDB_LITE

  virtual Status DisableFileDeletions() override {
    return db_->DisableFileDeletions();
  }

  virtual Status EnableFileDeletions(bool force) override {
    return db_->EnableFileDeletions(force);
  }

  virtual void GetLiveFilesMetaData(
      std::vector<LiveFileMetaData>* metadata) override {
    db_->GetLiveFilesMetaData(metadata);
  }

  virtual void GetColumnFamilyMetaData(
      ColumnFamilyHandle *column_family,
      ColumnFamilyMetaData* cf_meta) override {
    db_->GetColumnFamilyMetaData(column_family, cf_meta);
  }

#endif  // ROCKSDB_LITE

  virtual Status GetLiveFiles(std::vector<std::string>& vec, uint64_t* mfs,
                              bool flush_memtable = true) override {
      return db_->GetLiveFiles(vec, mfs, flush_memtable);
  }

  virtual SequenceNumber GetLatestSequenceNumber() const override {
    return db_->GetLatestSequenceNumber();
  }

  virtual Status GetSortedWalFiles(VectorLogPtr& files) override {
    return db_->GetSortedWalFiles(files);
  }

  virtual Status DeleteFile(std::string name) override {
    return db_->DeleteFile(name);
  }

  virtual Status GetDbIdentity(std::string& identity) const override {
    return db_->GetDbIdentity(identity);
  }

  using DB::SetOptions;
  virtual Status SetOptions(
    const std::unordered_map<std::string, std::string>& new_options) override {
    return db_->SetOptions(new_options);
  }

  using DB::GetPropertiesOfAllTables;
  virtual Status GetPropertiesOfAllTables(
      ColumnFamilyHandle* column_family,
      TablePropertiesCollection* props) override {
    return db_->GetPropertiesOfAllTables(column_family, props);
  }

  virtual Status GetUpdatesSince(
      SequenceNumber seq_number, unique_ptr<TransactionLogIterator>* iter,
      const TransactionLogIterator::ReadOptions& read_options) override {
    return db_->GetUpdatesSince(seq_number, iter, read_options);
  }

  virtual ColumnFamilyHandle* DefaultColumnFamily() const override {
    return db_->DefaultColumnFamily();
  }

 protected:
  DB* db_;
};

} //  namespace rocksdb
#line 23 "/home/evan/source/rocksdb/include/rocksdb/utilities/backupable_db.h"


namespace rocksdb {

struct BackupableDBOptions {
  // Where to keep the backup files. Has to be different than dbname_
  // Best to set this to dbname_ + "/backups"
  // Required
  std::string backup_dir;

  // Backup Env object. It will be used for backup file I/O. If it's
  // nullptr, backups will be written out using DBs Env. If it's
  // non-nullptr, backup's I/O will be performed using this object.
  // If you want to have backups on HDFS, use HDFS Env here!
  // Default: nullptr
  Env* backup_env;

  // If share_table_files == true, backup will assume that table files with
  // same name have the same contents. This enables incremental backups and
  // avoids unnecessary data copies.
  // If share_table_files == false, each backup will be on its own and will
  // not share any data with other backups.
  // default: true
  bool share_table_files;

  // Backup info and error messages will be written to info_log
  // if non-nullptr.
  // Default: nullptr
  Logger* info_log;

  // If sync == true, we can guarantee you'll get consistent backup even
  // on a machine crash/reboot. Backup process is slower with sync enabled.
  // If sync == false, we don't guarantee anything on machine reboot. However,
  // chances are some of the backups are consistent.
  // Default: true
  bool sync;

  // If true, it will delete whatever backups there are already
  // Default: false
  bool destroy_old_data;

  // If false, we won't backup log files. This option can be useful for backing
  // up in-memory databases where log file are persisted, but table files are in
  // memory.
  // Default: true
  bool backup_log_files;

  // Max bytes that can be transferred in a second during backup.
  // If 0, go as fast as you can
  // Default: 0
  uint64_t backup_rate_limit;

  // Max bytes that can be transferred in a second during restore.
  // If 0, go as fast as you can
  // Default: 0
  uint64_t restore_rate_limit;

  // Only used if share_table_files is set to true. If true, will consider that
  // backups can come from different databases, hence a sst is not uniquely
  // identifed by its name, but by the triple (file name, crc32, file length)
  // Default: false
  // Note: this is an experimental option, and you'll need to set it manually
  // *turn it on only if you know what you're doing*
  bool share_files_with_checksum;

  // Up to this many background threads will copy files for CreateNewBackup()
  // and RestoreDBFromBackup()
  // Default: 1
  int max_background_operations;

  // During backup user can get callback every time next
  // callback_trigger_interval_size bytes being copied.
  // Default: 4194304
  uint64_t callback_trigger_interval_size;

  void Dump(Logger* logger) const;

  explicit BackupableDBOptions(
      const std::string& _backup_dir, Env* _backup_env = nullptr,
      bool _share_table_files = true, Logger* _info_log = nullptr,
      bool _sync = true, bool _destroy_old_data = false,
      bool _backup_log_files = true, uint64_t _backup_rate_limit = 0,
      uint64_t _restore_rate_limit = 0, int _max_background_operations = 1,
      uint64_t _callback_trigger_interval_size = 4 * 1024 * 1024)
      : backup_dir(_backup_dir),
        backup_env(_backup_env),
        share_table_files(_share_table_files),
        info_log(_info_log),
        sync(_sync),
        destroy_old_data(_destroy_old_data),
        backup_log_files(_backup_log_files),
        backup_rate_limit(_backup_rate_limit),
        restore_rate_limit(_restore_rate_limit),
        share_files_with_checksum(false),
        max_background_operations(_max_background_operations),
        callback_trigger_interval_size(_callback_trigger_interval_size) {
    assert(share_table_files || !share_files_with_checksum);
  }
};

struct RestoreOptions {
  // If true, restore won't overwrite the existing log files in wal_dir. It will
  // also move all log files from archive directory to wal_dir. Use this option
  // in combination with BackupableDBOptions::backup_log_files = false for
  // persisting in-memory databases.
  // Default: false
  bool keep_log_files;

  explicit RestoreOptions(bool _keep_log_files = false)
      : keep_log_files(_keep_log_files) {}
};

typedef uint32_t BackupID;

struct BackupInfo {
  BackupID backup_id;
  int64_t timestamp;
  uint64_t size;

  uint32_t number_files;

  BackupInfo() {}

  BackupInfo(BackupID _backup_id, int64_t _timestamp, uint64_t _size,
             uint32_t _number_files)
      : backup_id(_backup_id), timestamp(_timestamp), size(_size),
        number_files(_number_files) {}
};

class BackupStatistics {
 public:
  BackupStatistics() {
    number_success_backup = 0;
    number_fail_backup = 0;
  }

  BackupStatistics(uint32_t _number_success_backup,
                   uint32_t _number_fail_backup)
      : number_success_backup(_number_success_backup),
        number_fail_backup(_number_fail_backup) {}

  ~BackupStatistics() {}

  void IncrementNumberSuccessBackup();
  void IncrementNumberFailBackup();

  uint32_t GetNumberSuccessBackup() const;
  uint32_t GetNumberFailBackup() const;

  std::string ToString() const;

 private:
  uint32_t number_success_backup;
  uint32_t number_fail_backup;
};

class BackupEngineReadOnly {
 public:
  virtual ~BackupEngineReadOnly() {}

  static Status Open(Env* db_env, const BackupableDBOptions& options,
                     BackupEngineReadOnly** backup_engine_ptr);

  // You can GetBackupInfo safely, even with other BackupEngine performing
  // backups on the same directory
  virtual void GetBackupInfo(std::vector<BackupInfo>* backup_info) = 0;
  virtual void GetCorruptedBackups(
      std::vector<BackupID>* corrupt_backup_ids) = 0;

  // Restoring DB from backup is NOT safe when there is another BackupEngine
  // running that might call DeleteBackup() or PurgeOldBackups(). It is caller's
  // responsibility to synchronize the operation, i.e. don't delete the backup
  // when you're restoring from it
  virtual Status RestoreDBFromBackup(
      BackupID backup_id, const std::string& db_dir, const std::string& wal_dir,
      const RestoreOptions& restore_options = RestoreOptions()) = 0;
  virtual Status RestoreDBFromLatestBackup(
      const std::string& db_dir, const std::string& wal_dir,
      const RestoreOptions& restore_options = RestoreOptions()) = 0;

  // checks that each file exists and that the size of the file matches our
  // expectations. it does not check file checksum.
  // Returns Status::OK() if all checks are good
  virtual Status VerifyBackup(BackupID backup_id) = 0;
};

// Please see the documentation in BackupableDB and RestoreBackupableDB
class BackupEngine {
 public:
  virtual ~BackupEngine() {}

  static Status Open(Env* db_env,
                     const BackupableDBOptions& options,
                     BackupEngine** backup_engine_ptr);

  virtual Status CreateNewBackup(
      DB* db, bool flush_before_backup = false,
      std::function<void()> progress_callback = []() {}) = 0;
  virtual Status PurgeOldBackups(uint32_t num_backups_to_keep) = 0;
  virtual Status DeleteBackup(BackupID backup_id) = 0;
  virtual void StopBackup() = 0;

  virtual void GetBackupInfo(std::vector<BackupInfo>* backup_info) = 0;
  virtual void GetCorruptedBackups(
      std::vector<BackupID>* corrupt_backup_ids) = 0;
  virtual Status RestoreDBFromBackup(
      BackupID backup_id, const std::string& db_dir, const std::string& wal_dir,
      const RestoreOptions& restore_options = RestoreOptions()) = 0;
  virtual Status RestoreDBFromLatestBackup(
      const std::string& db_dir, const std::string& wal_dir,
      const RestoreOptions& restore_options = RestoreOptions()) = 0;

  // checks that each file exists and that the size of the file matches our
  // expectations. it does not check file checksum.
  // Returns Status::OK() if all checks are good
  virtual Status VerifyBackup(BackupID backup_id) = 0;

  virtual Status GarbageCollect() = 0;
};

// Stack your DB with BackupableDB to be able to backup the DB
class BackupableDB : public StackableDB {
 public:
  // BackupableDBOptions have to be the same as the ones used in a previous
  // incarnation of the DB
  //
  // BackupableDB ownes the pointer `DB* db` now. You should not delete it or
  // use it after the invocation of BackupableDB
  BackupableDB(DB* db, const BackupableDBOptions& options);
  virtual ~BackupableDB();

  // Captures the state of the database in the latest backup
  // NOT a thread safe call
  Status CreateNewBackup(bool flush_before_backup = false);
  // Returns info about backups in backup_info
  void GetBackupInfo(std::vector<BackupInfo>* backup_info);
  // Returns info about corrupt backups in corrupt_backups
  void GetCorruptedBackups(std::vector<BackupID>* corrupt_backup_ids);
  // deletes old backups, keeping latest num_backups_to_keep alive
  Status PurgeOldBackups(uint32_t num_backups_to_keep);
  // deletes a specific backup
  Status DeleteBackup(BackupID backup_id);
  // Call this from another thread if you want to stop the backup
  // that is currently happening. It will return immediatelly, will
  // not wait for the backup to stop.
  // The backup will stop ASAP and the call to CreateNewBackup will
  // return Status::Incomplete(). It will not clean up after itself, but
  // the state will remain consistent. The state will be cleaned up
  // next time you create BackupableDB or RestoreBackupableDB.
  void StopBackup();

  // Will delete all the files we don't need anymore
  // It will do the full scan of the files/ directory and delete all the
  // files that are not referenced.
  Status GarbageCollect();

 private:
  BackupEngine* backup_engine_;
  Status status_;
};

// Use this class to access information about backups and restore from them
class RestoreBackupableDB {
 public:
  RestoreBackupableDB(Env* db_env, const BackupableDBOptions& options);
  ~RestoreBackupableDB();

  // Returns info about backups in backup_info
  void GetBackupInfo(std::vector<BackupInfo>* backup_info);
  // Returns info about corrupt backups in corrupt_backups
  void GetCorruptedBackups(std::vector<BackupID>* corrupt_backup_ids);

  // restore from backup with backup_id
  // IMPORTANT -- if options_.share_table_files == true and you restore DB
  // from some backup that is not the latest, and you start creating new
  // backups from the new DB, they will probably fail
  //
  // Example: Let's say you have backups 1, 2, 3, 4, 5 and you restore 3.
  // If you add new data to the DB and try creating a new backup now, the
  // database will diverge from backups 4 and 5 and the new backup will fail.
  // If you want to create new backup, you will first have to delete backups 4
  // and 5.
  Status RestoreDBFromBackup(BackupID backup_id, const std::string& db_dir,
                             const std::string& wal_dir,
                             const RestoreOptions& restore_options =
                                 RestoreOptions());

  // restore from the latest backup
  Status RestoreDBFromLatestBackup(const std::string& db_dir,
                                   const std::string& wal_dir,
                                   const RestoreOptions& restore_options =
                                       RestoreOptions());
  // deletes old backups, keeping latest num_backups_to_keep alive
  Status PurgeOldBackups(uint32_t num_backups_to_keep);
  // deletes a specific backup
  Status DeleteBackup(BackupID backup_id);

  // Will delete all the files we don't need anymore
  // It will do the full scan of the files/ directory and delete all the
  // files that are not referenced.
  Status GarbageCollect();

 private:
  BackupEngine* backup_engine_;
  Status status_;
};

}  // namespace rocksdb
#endif  // ROCKSDB_LITE
#line 1 "/home/evan/source/rocksdb/include/rocksdb/perf_context.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#ifndef STORAGE_ROCKSDB_INCLUDE_PERF_CONTEXT_H
#define STORAGE_ROCKSDB_INCLUDE_PERF_CONTEXT_H

#include <stdint.h>
#include <string>


namespace rocksdb {

// A thread local context for gathering performance counter efficiently
// and transparently.
// Use SetPerfLevel(PerfLevel::kEnableTime) to enable time stats.

struct PerfContext {

  void Reset(); // reset all performance counters to zero

  std::string ToString() const;

  uint64_t user_key_comparison_count; // total number of user key comparisons
  uint64_t block_cache_hit_count;     // total number of block cache hits
  uint64_t block_read_count;          // total number of block reads (with IO)
  uint64_t block_read_byte;           // total number of bytes from block reads
  uint64_t block_read_time;           // total time spent on block reads
  uint64_t block_checksum_time;       // total time spent on block checksum
  uint64_t block_decompress_time;     // total time spent on block decompression
  // total number of internal keys skipped over during iteration (overwritten or
  // deleted, to be more specific, hidden by a put or delete of the same key)
  uint64_t internal_key_skipped_count;
  // total number of deletes and single deletes skipped over during iteration
  uint64_t internal_delete_skipped_count;

  uint64_t get_snapshot_time;          // total time spent on getting snapshot
  uint64_t get_from_memtable_time;     // total time spent on querying memtables
  uint64_t get_from_memtable_count;    // number of mem tables queried
  // total time spent after Get() finds a key
  uint64_t get_post_process_time;
  uint64_t get_from_output_files_time; // total time reading from output files
  // total time spent on seeking memtable
  uint64_t seek_on_memtable_time;
  // number of seeks issued on memtable
  uint64_t seek_on_memtable_count;
  // total time spent on seeking child iters
  uint64_t seek_child_seek_time;
  // number of seek issued in child iterators
  uint64_t seek_child_seek_count;
  uint64_t seek_min_heap_time;         // total time spent on the merge heap
  // total time spent on seeking the internal entries
  uint64_t seek_internal_seek_time;
  // total time spent on iterating internal entries to find the next user entry
  uint64_t find_next_user_entry_time;

  // total time spent on writing to WAL
  uint64_t write_wal_time;
  // total time spent on writing to mem tables
  uint64_t write_memtable_time;
  // total time spent on delaying write
  uint64_t write_delay_time;
  // total time spent on writing a record, excluding the above three times
  uint64_t write_pre_and_post_process_time;

  uint64_t db_mutex_lock_nanos;      // time spent on acquiring DB mutex.
  // Time spent on waiting with a condition variable created with DB mutex.
  uint64_t db_condition_wait_nanos;
  // Time spent on merge operator.
  uint64_t merge_operator_time_nanos;

  // Time spent on reading index block from block cache or SST file
  uint64_t read_index_block_nanos;
  // Time spent on reading filter block from block cache or SST file
  uint64_t read_filter_block_nanos;
  // Time spent on creating data block iterator
  uint64_t new_table_block_iter_nanos;
  // Time spent on creating a iterator of an SST file.
  uint64_t new_table_iterator_nanos;
  // Time spent on seeking a key in data/index blocks
  uint64_t block_seek_nanos;
  // Time spent on finding or creating a table reader
  uint64_t find_table_nanos;
  // total number of mem table bloom hits
  uint64_t bloom_memtable_hit_count;
  // total number of mem table bloom misses
  uint64_t bloom_memtable_miss_count;
  // total number of SST table bloom hits
  uint64_t bloom_sst_hit_count;
  // total number of SST table bloom misses
  uint64_t bloom_sst_miss_count;
};

#if defined(NPERF_CONTEXT) || defined(IOS_CROSS_COMPILE)
extern PerfContext perf_context;
#elif _WIN32
extern __declspec(thread) PerfContext perf_context;
#else
extern __thread PerfContext perf_context;
#endif

}

#endif
#line 1 "/home/evan/source/rocksdb/include/rocksdb/delete_scheduler.h"
//  Copyright (c) 2015, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.


#include <map>
#include <string>


namespace rocksdb {

class Env;
class Logger;

// DeleteScheduler allow the DB to enforce a rate limit on file deletion,
// Instead of deleteing files immediately, files are moved to trash_dir
// and deleted in a background thread that apply sleep penlty between deletes
// if they are happening in a rate faster than rate_bytes_per_sec,
//
// Rate limiting can be turned off by setting rate_bytes_per_sec = 0, In this
// case DeleteScheduler will delete files immediately.
class DeleteScheduler {
 public:
  virtual ~DeleteScheduler() {}

  // Return delete rate limit in bytes per second
  virtual int64_t GetRateBytesPerSecond() = 0;

  // Move file to trash directory and schedule it's deletion
  virtual Status DeleteFile(const std::string& fname) = 0;

  // Return a map containing errors that happened in the background thread
  // file_path => error status
  virtual std::map<std::string, Status> GetBackgroundErrors() = 0;

  // Wait for all files being deleteing in the background to finish or for
  // destructor to be called.
  virtual void WaitForEmptyTrash() = 0;
};

// Create a new DeleteScheduler that can be shared among multiple RocksDB
// instances to control the file deletion rate.
//
// @env: Pointer to Env object, please see "rocksdb/env.h".
// @trash_dir: Path to the directory where deleted files will be moved into
//    to be deleted in a background thread while applying rate limiting. If this
//    directory dont exist, it will be created. This directory should not be
//    used by any other process or any other DeleteScheduler.
// @rate_bytes_per_sec: How many bytes should be deleted per second, If this
//    value is set to 1024 (1 Kb / sec) and we deleted a file of size 4 Kb
//    in 1 second, we will wait for another 3 seconds before we delete other
//    files, Set to 0 to disable rate limiting.
// @info_log: If not nullptr, info_log will be used to log errors.
// @delete_exisitng_trash: If set to true, the newly created DeleteScheduler
//    will delete files that already exist in trash_dir.
// @status: If not nullptr, status will contain any errors that happened during
//    creating the missing trash_dir or deleting existing files in trash.
extern DeleteScheduler* NewDeleteScheduler(
    Env* env, const std::string& trash_dir, int64_t rate_bytes_per_sec,
    std::shared_ptr<Logger> info_log = nullptr,
    bool delete_exisitng_trash = true, Status* status = nullptr);

}  // namespace rocksdb
#line 1 "/home/evan/source/rocksdb/include/rocksdb/sst_file_writer.h"
//  Copyright (c) 2015, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.

#include <string>

namespace rocksdb {

class Comparator;

// Table Properties that are specific to tables created by SstFileWriter.
struct ExternalSstFilePropertyNames {
  // value of this property is a fixed int32 number.
  static const std::string kVersion;
};

// ExternalSstFileInfo include information about sst files created
// using SstFileWriter
struct ExternalSstFileInfo {
  ExternalSstFileInfo() {}
  ExternalSstFileInfo(const std::string& _file_path,
                      const std::string& _smallest_key,
                      const std::string& _largest_key,
                      SequenceNumber _sequence_number, uint64_t _file_size,
                      int32_t _num_entries, int32_t _version)
      : file_path(_file_path),
        smallest_key(_smallest_key),
        largest_key(_largest_key),
        sequence_number(_sequence_number),
        file_size(_file_size),
        num_entries(_num_entries),
        version(_version) {}

  std::string file_path;           // external sst file path
  std::string smallest_key;        // smallest user key in file
  std::string largest_key;         // largest user key in file
  SequenceNumber sequence_number;  // sequence number of all keys in file
  uint64_t file_size;              // file size in bytes
  uint64_t num_entries;            // number of entries in file
  int32_t version;                 // file version
};

// SstFileWriter is used to create sst files that can be added to database later
// All keys in files generated by SstFileWriter will have sequence number = 0
class SstFileWriter {
 public:
  SstFileWriter(const EnvOptions& env_options,
                const ImmutableCFOptions& ioptions,
                const Comparator* user_comparator);

  ~SstFileWriter();

  // Prepare SstFileWriter to write into file located at "file_path".
  Status Open(const std::string& file_path);

  // Add key, value to currently opened file
  // REQUIRES: key is after any previously added key according to comparator.
  Status Add(const Slice& user_key, const Slice& value);

  // Finalize writing to sst file and close file.
  //
  // An optional ExternalSstFileInfo pointer can be passed to the function
  // which will be populated with information about the created sst file
  Status Finish(ExternalSstFileInfo* file_info = nullptr);

 private:
  class SstFileWriterPropertiesCollectorFactory;
  class SstFileWriterPropertiesCollector;
  struct Rep;
  Rep* rep_;
};
}  // namespace rocksdb
#line 1 "/home/evan/source/rocksdb/include/rocksdb/experimental.h"
// Copyright (c) 2014, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.



namespace rocksdb {
namespace experimental {

// Supported only for Leveled compaction
Status SuggestCompactRange(DB* db, ColumnFamilyHandle* column_family,
                           const Slice* begin, const Slice* end);
Status SuggestCompactRange(DB* db, const Slice* begin, const Slice* end);

// Move all L0 files to target_level skipping compaction.
// This operation succeeds only if the files in L0 have disjoint ranges; this
// is guaranteed to happen, for instance, if keys are inserted in sorted
// order. Furthermore, all levels between 1 and target_level must be empty.
// If any of the above condition is violated, InvalidArgument will be
// returned.
Status PromoteL0(DB* db, ColumnFamilyHandle* column_family,
                 int target_level = 1);

}  // namespace experimental
}  // namespace rocksdb
#line 1 "/home/evan/source/rocksdb/include/rocksdb/db_dump_tool.h"
//  Copyright (c) 2015, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.

#ifndef ROCKSDB_LITE

#include <string>


namespace rocksdb {

struct DumpOptions {
  // Database that will be dumped
  std::string db_path;
  // File location that will contain dump output
  std::string dump_location;
  // Dont include db information header in the dump
  bool anonymous = false;
};

class DbDumpTool {
 public:
  bool Run(const DumpOptions& dump_options,
           rocksdb::Options options = rocksdb::Options());
};

struct UndumpOptions {
  // Database that we will load the dumped file into
  std::string db_path;
  // File location of the dumped file that will be loaded
  std::string dump_location;
  // Compact the db after loading the dumped file
  bool compact_db = false;
};

class DbUndumpTool {
 public:
  bool Run(const UndumpOptions& undump_options,
           rocksdb::Options options = rocksdb::Options());
};
}  // namespace rocksdb
#endif  // ROCKSDB_LITE
#line 1 "/home/evan/source/rocksdb/include/rocksdb/rate_limiter.h"
//  Copyright (c) 2014, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.
//
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.



namespace rocksdb {

class RateLimiter {
 public:
  virtual ~RateLimiter() {}

  // This API allows user to dynamically change rate limiter's bytes per second.
  // REQUIRED: bytes_per_second > 0
  virtual void SetBytesPerSecond(int64_t bytes_per_second) = 0;

  // Request for token to write bytes. If this request can not be satisfied,
  // the call is blocked. Caller is responsible to make sure
  // bytes <= GetSingleBurstBytes()
  virtual void Request(const int64_t bytes, const Env::IOPriority pri) = 0;

  // Max bytes can be granted in a single burst
  virtual int64_t GetSingleBurstBytes() const = 0;

  // Total bytes that go though rate limiter
  virtual int64_t GetTotalBytesThrough(
      const Env::IOPriority pri = Env::IO_TOTAL) const = 0;

  // Total # of requests that go though rate limiter
  virtual int64_t GetTotalRequests(
      const Env::IOPriority pri = Env::IO_TOTAL) const = 0;
};

// Create a RateLimiter object, which can be shared among RocksDB instances to
// control write rate of flush and compaction.
// @rate_bytes_per_sec: this is the only parameter you want to set most of the
// time. It controls the total write rate of compaction and flush in bytes per
// second. Currently, RocksDB does not enforce rate limit for anything other
// than flush and compaction, e.g. write to WAL.
// @refill_period_us: this controls how often tokens are refilled. For example,
// when rate_bytes_per_sec is set to 10MB/s and refill_period_us is set to
// 100ms, then 1MB is refilled every 100ms internally. Larger value can lead to
// burstier writes while smaller value introduces more CPU overhead.
// The default should work for most cases.
// @fairness: RateLimiter accepts high-pri requests and low-pri requests.
// A low-pri request is usually blocked in favor of hi-pri request. Currently,
// RocksDB assigns low-pri to request from compaciton and high-pri to request
// from flush. Low-pri requests can get blocked if flush requests come in
// continuouly. This fairness parameter grants low-pri requests permission by
// 1/fairness chance even though high-pri requests exist to avoid starvation.
// You should be good by leaving it at default 10.
extern RateLimiter* NewGenericRateLimiter(
    int64_t rate_bytes_per_sec,
    int64_t refill_period_us = 100 * 1000,
    int32_t fairness = 10);

}  // namespace rocksdb
#line 1 "/home/evan/source/rocksdb/include/rocksdb/utilities/info_log_finder.h"
// Copyright (c) 2014, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.


#include <string>
#include <vector>


namespace rocksdb {

// This function can be used to list the Information logs,
// given the db pointer.
Status GetInfoLogList(DB* db, std::vector<std::string>* info_log_list);
}  // namespace rocksdb
#line 1 "/home/evan/source/rocksdb/include/rocksdb/utilities/checkpoint.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
//
// A checkpoint is an openable snapshot of a database at a point in time.

#ifndef ROCKSDB_LITE

#include <string>

namespace rocksdb {

class DB;

class Checkpoint {
 public:
  // Creates a Checkpoint object to be used for creating openable sbapshots
  static Status Create(DB* db, Checkpoint** checkpoint_ptr);

  // Builds an openable snapshot of RocksDB on the same disk, which
  // accepts an output directory on the same disk, and under the directory
  // (1) hard-linked SST files pointing to existing live SST files
  // SST files will be copied if output directory is on a different filesystem
  // (2) a copied manifest files and other files
  // The directory should not already exist and will be created by this API.
  // The directory will be an absolute path
  virtual Status CreateCheckpoint(const std::string& checkpoint_dir);

  virtual ~Checkpoint() {}
};

}  // namespace rocksdb
#endif  // !ROCKSDB_LITE
#line 1 "/home/evan/source/rocksdb/include/rocksdb/utilities/document_db.h"
//  Copyright (c) 2013, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.

#ifndef ROCKSDB_LITE

#include <string>
#include <vector>

#line 1 "/home/evan/source/rocksdb/include/rocksdb/utilities/json_document.h"
//  Copyright (c) 2013, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.
#ifndef ROCKSDB_LITE

#include <deque>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>


// We use JSONDocument for DocumentDB API
// Implementation inspired by folly::dynamic, rapidjson and fbson

namespace fbson {
  class FbsonValue;
  class ObjectVal;
  template <typename T>
  class FbsonWriterT;
  class FbsonOutStream;
  typedef FbsonWriterT<FbsonOutStream> FbsonWriter;
}  // namespace fbson

namespace rocksdb {

// NOTE: none of this is thread-safe
class JSONDocument {
 public:
  // return nullptr on parse failure
  static JSONDocument* ParseJSON(const char* json);

  enum Type {
    kNull,
    kArray,
    kBool,
    kDouble,
    kInt64,
    kObject,
    kString,
  };

  /* implicit */ JSONDocument();  // null
  /* implicit */ JSONDocument(bool b);
  /* implicit */ JSONDocument(double d);
  /* implicit */ JSONDocument(int8_t i);
  /* implicit */ JSONDocument(int16_t i);
  /* implicit */ JSONDocument(int32_t i);
  /* implicit */ JSONDocument(int64_t i);
  /* implicit */ JSONDocument(const std::string& s);
  /* implicit */ JSONDocument(const char* s);
  // constructs JSONDocument of specific type with default value
  explicit JSONDocument(Type _type);

  JSONDocument(const JSONDocument& json_document);

  JSONDocument(JSONDocument&& json_document);

  Type type() const;

  // REQUIRES: IsObject()
  bool Contains(const std::string& key) const;
  // REQUIRES: IsObject()
  // Returns non-owner object
  JSONDocument operator[](const std::string& key) const;

  // REQUIRES: IsArray() == true || IsObject() == true
  size_t Count() const;

  // REQUIRES: IsArray()
  // Returns non-owner object
  JSONDocument operator[](size_t i) const;

  JSONDocument& operator=(JSONDocument jsonDocument);

  bool IsNull() const;
  bool IsArray() const;
  bool IsBool() const;
  bool IsDouble() const;
  bool IsInt64() const;
  bool IsObject() const;
  bool IsString() const;

  // REQUIRES: IsBool() == true
  bool GetBool() const;
  // REQUIRES: IsDouble() == true
  double GetDouble() const;
  // REQUIRES: IsInt64() == true
  int64_t GetInt64() const;
  // REQUIRES: IsString() == true
  std::string GetString() const;

  bool operator==(const JSONDocument& rhs) const;

  bool operator!=(const JSONDocument& rhs) const;

  JSONDocument Copy() const;

  bool IsOwner() const;

  std::string DebugString() const;

 private:
  class ItemsIteratorGenerator;

 public:
  // REQUIRES: IsObject()
  ItemsIteratorGenerator Items() const;

  // appends serialized object to dst
  void Serialize(std::string* dst) const;
  // returns nullptr if Slice doesn't represent valid serialized JSONDocument
  static JSONDocument* Deserialize(const Slice& src);

 private:
  friend class JSONDocumentBuilder;

  JSONDocument(fbson::FbsonValue* val, bool makeCopy);

  void InitFromValue(const fbson::FbsonValue* val);

  // iteration on objects
  class const_item_iterator {
   private:
    class Impl;
   public:
    typedef std::pair<std::string, JSONDocument> value_type;
    explicit const_item_iterator(Impl* impl);
    const_item_iterator(const_item_iterator&&);
    const_item_iterator& operator++();
    bool operator!=(const const_item_iterator& other);
    value_type operator*();
    ~const_item_iterator();
   private:
    friend class ItemsIteratorGenerator;
    std::unique_ptr<Impl> it_;
  };

  class ItemsIteratorGenerator {
   public:
    explicit ItemsIteratorGenerator(const fbson::ObjectVal& object);
    const_item_iterator begin() const;

    const_item_iterator end() const;

   private:
    const fbson::ObjectVal& object_;
  };

  std::unique_ptr<char[]> data_;
  mutable fbson::FbsonValue* value_;

  // Our serialization format's first byte specifies the encoding version. That
  // way, we can easily change our format while providing backwards
  // compatibility. This constant specifies the current version of the
  // serialization format
  static const char kSerializationFormatVersion;
};

class JSONDocumentBuilder {
 public:
  JSONDocumentBuilder();

  explicit JSONDocumentBuilder(fbson::FbsonOutStream* out);

  void Reset();

  bool WriteStartArray();

  bool WriteEndArray();

  bool WriteStartObject();

  bool WriteEndObject();

  bool WriteKeyValue(const std::string& key, const JSONDocument& value);

  bool WriteJSONDocument(const JSONDocument& value);

  JSONDocument GetJSONDocument();

  ~JSONDocumentBuilder();

 private:
  std::unique_ptr<fbson::FbsonWriter> writer_;
};

}  // namespace rocksdb

#endif  // ROCKSDB_LITE
#line 13 "/home/evan/source/rocksdb/include/rocksdb/utilities/document_db.h"

namespace rocksdb {

// IMPORTANT: DocumentDB is a work in progress. It is unstable and we might
// change the API without warning. Talk to RocksDB team before using this in
// production ;)

// DocumentDB is a layer on top of RocksDB that provides a very simple JSON API.
// When creating a DB, you specify a list of indexes you want to keep on your
// data. You can insert a JSON document to the DB, which is automatically
// indexed. Every document added to the DB needs to have "_id" field which is
// automatically indexed and is an unique primary key. All other indexes are
// non-unique.

// NOTE: field names in the JSON are NOT allowed to start with '$' or
// contain '.'. We don't currently enforce that rule, but will start behaving
// badly.

// Cursor is what you get as a result of executing query. To get all
// results from a query, call Next() on a Cursor while  Valid() returns true
class Cursor {
 public:
  Cursor() = default;
  virtual ~Cursor() {}

  virtual bool Valid() const = 0;
  virtual void Next() = 0;
  // Lifecycle of the returned JSONDocument is until the next Next() call
  virtual const JSONDocument& document() const = 0;
  virtual Status status() const = 0;

 private:
  // No copying allowed
  Cursor(const Cursor&);
  void operator=(const Cursor&);
};

struct DocumentDBOptions {
  int background_threads = 4;
  uint64_t memtable_size = 128 * 1024 * 1024;    // 128 MB
  uint64_t cache_size = 1 * 1024 * 1024 * 1024;  // 1 GB
};

// TODO(icanadi) Add `JSONDocument* info` parameter to all calls that can be
// used by the caller to get more information about the call execution (number
// of dropped records, number of updated records, etc.)
class DocumentDB : public StackableDB {
 public:
  struct IndexDescriptor {
    // Currently, you can only define an index on a single field. To specify an
    // index on a field X, set index description to JSON "{X: 1}"
    // Currently the value needs to be 1, which means ascending.
    // In the future, we plan to also support indexes on multiple keys, where
    // you could mix ascending sorting (1) with descending sorting indexes (-1)
    JSONDocument* description;
    std::string name;
  };

  // Open DocumentDB with specified indexes. The list of indexes has to be
  // complete, i.e. include all indexes present in the DB, except the primary
  // key index.
  // Otherwise, Open() will return an error
  static Status Open(const DocumentDBOptions& options, const std::string& name,
                     const std::vector<IndexDescriptor>& indexes,
                     DocumentDB** db, bool read_only = false);

  explicit DocumentDB(DB* db) : StackableDB(db) {}

  // Create a new index. It will stop all writes for the duration of the call.
  // All current documents in the DB are scanned and corresponding index entries
  // are created
  virtual Status CreateIndex(const WriteOptions& write_options,
                             const IndexDescriptor& index) = 0;

  // Drop an index. Client is responsible to make sure that index is not being
  // used by currently executing queries
  virtual Status DropIndex(const std::string& name) = 0;

  // Insert a document to the DB. The document needs to have a primary key "_id"
  // which can either be a string or an integer. Otherwise the write will fail
  // with InvalidArgument.
  virtual Status Insert(const WriteOptions& options,
                        const JSONDocument& document) = 0;

  // Deletes all documents matching a filter atomically
  virtual Status Remove(const ReadOptions& read_options,
                        const WriteOptions& write_options,
                        const JSONDocument& query) = 0;

  // Does this sequence of operations:
  // 1. Find all documents matching a filter
  // 2. For all documents, atomically:
  // 2.1. apply the update operators
  // 2.2. update the secondary indexes
  //
  // Currently only $set update operator is supported.
  // Syntax is: {$set: {key1: value1, key2: value2, etc...}}
  // This operator will change a document's key1 field to value1, key2 to
  // value2, etc. New values will be set even if a document didn't have an entry
  // for the specified key.
  //
  // You can not change a primary key of a document.
  //
  // Update example: Update({id: {$gt: 5}, $index: id}, {$set: {enabled: true}})
  virtual Status Update(const ReadOptions& read_options,
                        const WriteOptions& write_options,
                        const JSONDocument& filter,
                        const JSONDocument& updates) = 0;

  // query has to be an array in which every element is an operator. Currently
  // only $filter operator is supported. Syntax of $filter operator is:
  // {$filter: {key1: condition1, key2: condition2, etc.}} where conditions can
  // be either:
  // 1) a single value in which case the condition is equality condition, or
  // 2) a defined operators, like {$gt: 4}, which will match all documents that
  // have key greater than 4.
  //
  // Supported operators are:
  // 1) $gt -- greater than
  // 2) $gte -- greater than or equal
  // 3) $lt -- less than
  // 4) $lte -- less than or equal
  // If you want the filter to use an index, you need to specify it like this:
  // {$filter: {...(conditions)..., $index: index_name}}
  //
  // Example query:
  // * [{$filter: {name: John, age: {$gte: 18}, $index: age}}]
  // will return all Johns whose age is greater or equal to 18 and it will use
  // index "age" to satisfy the query.
  virtual Cursor* Query(const ReadOptions& read_options,
                        const JSONDocument& query) = 0;
};

}  // namespace rocksdb
#endif  // ROCKSDB_LITE
#line 1 "/home/evan/source/rocksdb/include/rocksdb/utilities/flashcache.h"
// Copyright (c) 2014, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.


#include <string>

namespace rocksdb {

// This API is experimental. We will mark it stable once we run it in production
// for a while.
// NewFlashcacheAwareEnv() creates and Env that blacklists all background
// threads (used for flush and compaction) from using flashcache to cache their
// reads. Reads from compaction thread don't need to be cached because they are
// going to be soon made obsolete (due to nature of compaction)
// Usually you would pass Env::Default() as base.
// cachedev_fd is a file descriptor of the flashcache device. Caller has to
// open flashcache device before calling this API.
extern std::unique_ptr<Env> NewFlashcacheAwareEnv(
    Env* base, const int cachedev_fd);

}  // namespace rocksdb
#line 1 "/home/evan/source/rocksdb/include/rocksdb/utilities/geo_db.h"
//  Copyright (c) 2013, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.
//

#ifndef ROCKSDB_LITE
#include <string>
#include <vector>


namespace rocksdb {

//
// Configurable options needed for setting up a Geo database
//
struct GeoDBOptions {
  // Backup info and error messages will be written to info_log
  // if non-nullptr.
  // Default: nullptr
  Logger* info_log;

  explicit GeoDBOptions(Logger* _info_log = nullptr):info_log(_info_log) { }
};

//
// A position in the earth's geoid
//
class GeoPosition {
 public:
  double latitude;
  double longitude;

  explicit GeoPosition(double la = 0, double lo = 0) :
    latitude(la), longitude(lo) {
  }
};

//
// Description of an object on the Geoid. It is located by a GPS location,
// and is identified by the id. The value associated with this object is
// an opaque string 'value'. Different objects identified by unique id's
// can have the same gps-location associated with them.
//
class GeoObject {
 public:
  GeoPosition position;
  std::string id;
  std::string value;

  GeoObject() {}

  GeoObject(const GeoPosition& pos, const std::string& i,
            const std::string& val) :
    position(pos), id(i), value(val) {
  }
};

//
// Stack your DB with GeoDB to be able to get geo-spatial support
//
class GeoDB : public StackableDB {
 public:
  // GeoDBOptions have to be the same as the ones used in a previous
  // incarnation of the DB
  //
  // GeoDB owns the pointer `DB* db` now. You should not delete it or
  // use it after the invocation of GeoDB
  // GeoDB(DB* db, const GeoDBOptions& options) : StackableDB(db) {}
  GeoDB(DB* db, const GeoDBOptions& options) : StackableDB(db) {}
  virtual ~GeoDB() {}

  // Insert a new object into the location database. The object is
  // uniquely identified by the id. If an object with the same id already
  // exists in the db, then the old one is overwritten by the new
  // object being inserted here.
  virtual Status Insert(const GeoObject& object) = 0;

  // Retrieve the value of the object located at the specified GPS
  // location and is identified by the 'id'.
  virtual Status GetByPosition(const GeoPosition& pos,
                               const Slice& id, std::string* value) = 0;

  // Retrieve the value of the object identified by the 'id'. This method
  // could be potentially slower than GetByPosition
  virtual Status GetById(const Slice& id, GeoObject*  object) = 0;

  // Delete the specified object
  virtual Status Remove(const Slice& id) = 0;

  // Returns a list of all items within a circular radius from the
  // specified gps location. If 'number_of_values' is specified,
  // then this call returns at most that many number of objects.
  // The radius is specified in 'meters'.
  virtual Status SearchRadial(const GeoPosition& pos,
                              double radius,
                              std::vector<GeoObject>* values,
                              int number_of_values = INT_MAX) = 0;
};

}  // namespace rocksdb
#endif  // ROCKSDB_LITE
#line 1 "/home/evan/source/rocksdb/include/rocksdb/utilities/leveldb_options.h"
//  Copyright (c) 2014, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.
//
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.


#include <stddef.h>

namespace rocksdb {

class Cache;
class Comparator;
class Env;
class FilterPolicy;
class Logger;
struct Options;
class Snapshot;

enum CompressionType : char;

// Options to control the behavior of a database (passed to
// DB::Open). A LevelDBOptions object can be initialized as though
// it were a LevelDB Options object, and then it can be converted into
// a RocksDB Options object.
struct LevelDBOptions {
  // -------------------
  // Parameters that affect behavior

  // Comparator used to define the order of keys in the table.
  // Default: a comparator that uses lexicographic byte-wise ordering
  //
  // REQUIRES: The client must ensure that the comparator supplied
  // here has the same name and orders keys *exactly* the same as the
  // comparator provided to previous open calls on the same DB.
  const Comparator* comparator;

  // If true, the database will be created if it is missing.
  // Default: false
  bool create_if_missing;

  // If true, an error is raised if the database already exists.
  // Default: false
  bool error_if_exists;

  // If true, the implementation will do aggressive checking of the
  // data it is processing and will stop early if it detects any
  // errors.  This may have unforeseen ramifications: for example, a
  // corruption of one DB entry may cause a large number of entries to
  // become unreadable or for the entire DB to become unopenable.
  // Default: false
  bool paranoid_checks;

  // Use the specified object to interact with the environment,
  // e.g. to read/write files, schedule background work, etc.
  // Default: Env::Default()
  Env* env;

  // Any internal progress/error information generated by the db will
  // be written to info_log if it is non-NULL, or to a file stored
  // in the same directory as the DB contents if info_log is NULL.
  // Default: NULL
  Logger* info_log;

  // -------------------
  // Parameters that affect performance

  // Amount of data to build up in memory (backed by an unsorted log
  // on disk) before converting to a sorted on-disk file.
  //
  // Larger values increase performance, especially during bulk loads.
  // Up to two write buffers may be held in memory at the same time,
  // so you may wish to adjust this parameter to control memory usage.
  // Also, a larger write buffer will result in a longer recovery time
  // the next time the database is opened.
  //
  // Default: 4MB
  size_t write_buffer_size;

  // Number of open files that can be used by the DB.  You may need to
  // increase this if your database has a large working set (budget
  // one open file per 2MB of working set).
  //
  // Default: 1000
  int max_open_files;

  // Control over blocks (user data is stored in a set of blocks, and
  // a block is the unit of reading from disk).

  // If non-NULL, use the specified cache for blocks.
  // If NULL, leveldb will automatically create and use an 8MB internal cache.
  // Default: NULL
  Cache* block_cache;

  // Approximate size of user data packed per block.  Note that the
  // block size specified here corresponds to uncompressed data.  The
  // actual size of the unit read from disk may be smaller if
  // compression is enabled.  This parameter can be changed dynamically.
  //
  // Default: 4K
  size_t block_size;

  // Number of keys between restart points for delta encoding of keys.
  // This parameter can be changed dynamically.  Most clients should
  // leave this parameter alone.
  //
  // Default: 16
  int block_restart_interval;

  // Compress blocks using the specified compression algorithm.  This
  // parameter can be changed dynamically.
  //
  // Default: kSnappyCompression, which gives lightweight but fast
  // compression.
  //
  // Typical speeds of kSnappyCompression on an Intel(R) Core(TM)2 2.4GHz:
  //    ~200-500MB/s compression
  //    ~400-800MB/s decompression
  // Note that these speeds are significantly faster than most
  // persistent storage speeds, and therefore it is typically never
  // worth switching to kNoCompression.  Even if the input data is
  // incompressible, the kSnappyCompression implementation will
  // efficiently detect that and will switch to uncompressed mode.
  CompressionType compression;

  // If non-NULL, use the specified filter policy to reduce disk reads.
  // Many applications will benefit from passing the result of
  // NewBloomFilterPolicy() here.
  //
  // Default: NULL
  const FilterPolicy* filter_policy;

  // Create a LevelDBOptions object with default values for all fields.
  LevelDBOptions();
};

// Converts a LevelDBOptions object into a RocksDB Options object.
Options ConvertOptions(const LevelDBOptions& leveldb_options);

}  // namespace rocksdb
#line 1 "/home/evan/source/rocksdb/include/rocksdb/utilities/spatial_db.h"
//  Copyright (c) 2013, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.

#ifndef ROCKSDB_LITE

#include <string>
#include <vector>


namespace rocksdb {
namespace spatial {

// NOTE: SpatialDB is experimental and we might change its API without warning.
// Please talk to us before developing against SpatialDB API.
//
// SpatialDB is a support for spatial indexes built on top of RocksDB.
// When creating a new SpatialDB, clients specifies a list of spatial indexes to
// build on their data. Each spatial index is defined by the area and
// granularity. If you're storing map data, different spatial index
// granularities can be used for different zoom levels.
//
// Each element inserted into SpatialDB has:
// * a bounding box, which determines how will the element be indexed
// * string blob, which will usually be WKB representation of the polygon
// (http://en.wikipedia.org/wiki/Well-known_text)
// * feature set, which is a map of key-value pairs, where value can be null,
// int, double, bool, string
// * a list of indexes to insert the element in
//
// Each query is executed on a single spatial index. Query guarantees that it
// will return all elements intersecting the specified bounding box, but it
// might also return some extra non-intersecting elements.

// Variant is a class that can be many things: null, bool, int, double or string
// It is used to store different value types in FeatureSet (see below)
struct Variant {
  // Don't change the values here, they are persisted on disk
  enum Type {
    kNull = 0x0,
    kBool = 0x1,
    kInt = 0x2,
    kDouble = 0x3,
    kString = 0x4,
  };

  Variant() : type_(kNull) {}
  /* implicit */ Variant(bool b) : type_(kBool) { data_.b = b; }
  /* implicit */ Variant(uint64_t i) : type_(kInt) { data_.i = i; }
  /* implicit */ Variant(double d) : type_(kDouble) { data_.d = d; }
  /* implicit */ Variant(const std::string& s) : type_(kString) {
    new (&data_.s) std::string(s);
  }

  Variant(const Variant& v) : type_(v.type_) { Init(v, data_); }

  Variant& operator=(const Variant& v);

  Variant(Variant&& rhs) : type_(kNull) { *this = std::move(rhs); }

  Variant& operator=(Variant&& v);

  ~Variant() { Destroy(type_, data_); }

  Type type() const { return type_; }
  bool get_bool() const { return data_.b; }
  uint64_t get_int() const { return data_.i; }
  double get_double() const { return data_.d; }
  const std::string& get_string() const { return *GetStringPtr(data_); }

  bool operator==(const Variant& other) const;
  bool operator!=(const Variant& other) const { return !(*this == other); }

 private:
  Type type_;

  union Data {
    bool b;
    uint64_t i;
    double d;
    // Current version of MS compiler not C++11 compliant so can not put
    // std::string
    // however, even then we still need the rest of the maintenance.
    char s[sizeof(std::string)];
  } data_;

  // Avoid type_punned aliasing problem
  static std::string* GetStringPtr(Data& d) {
    void* p = d.s;
    return reinterpret_cast<std::string*>(p);
  }

  static const std::string* GetStringPtr(const Data& d) {
    const void* p = d.s;
    return reinterpret_cast<const std::string*>(p);
  }

  static void Init(const Variant&, Data&);

  static void Destroy(Type t, Data& d) {
    if (t == kString) {
      using std::string;
      GetStringPtr(d)->~string();
    }
  }
};

// FeatureSet is a map of key-value pairs. One feature set is associated with
// each element in SpatialDB. It can be used to add rich data about the element.
class FeatureSet {
 private:
  typedef std::unordered_map<std::string, Variant> map;

 public:
  class iterator {
   public:
    /* implicit */ iterator(const map::const_iterator itr) : itr_(itr) {}
    iterator& operator++() {
      ++itr_;
      return *this;
    }
    bool operator!=(const iterator& other) { return itr_ != other.itr_; }
    bool operator==(const iterator& other) { return itr_ == other.itr_; }
    map::value_type operator*() { return *itr_; }

   private:
    map::const_iterator itr_;
  };
  FeatureSet() = default;

  FeatureSet* Set(const std::string& key, const Variant& value);
  bool Contains(const std::string& key) const;
  // REQUIRES: Contains(key)
  const Variant& Get(const std::string& key) const;
  iterator Find(const std::string& key) const;

  iterator begin() const { return map_.begin(); }
  iterator end() const { return map_.end(); }

  void Clear();
  size_t Size() const { return map_.size(); }

  void Serialize(std::string* output) const;
  // REQUIRED: empty FeatureSet
  bool Deserialize(const Slice& input);

  std::string DebugString() const;

 private:
  map map_;
};

// BoundingBox is a helper structure for defining rectangles representing
// bounding boxes of spatial elements.
template <typename T>
struct BoundingBox {
  T min_x, min_y, max_x, max_y;
  BoundingBox() = default;
  BoundingBox(T _min_x, T _min_y, T _max_x, T _max_y)
      : min_x(_min_x), min_y(_min_y), max_x(_max_x), max_y(_max_y) {}

  bool Intersects(const BoundingBox<T>& a) const {
    return !(min_x > a.max_x || min_y > a.max_y || a.min_x > max_x ||
             a.min_y > max_y);
  }
};

struct SpatialDBOptions {
  uint64_t cache_size = 1 * 1024 * 1024 * 1024LL;  // 1GB
  int num_threads = 16;
  bool bulk_load = true;
};

// Cursor is used to return data from the query to the client. To get all the
// data from the query, just call Next() while Valid() is true
class Cursor {
 public:
  Cursor() = default;
  virtual ~Cursor() {}

  virtual bool Valid() const = 0;
  // REQUIRES: Valid()
  virtual void Next() = 0;

  // Lifetime of the underlying storage until the next call to Next()
  // REQUIRES: Valid()
  virtual const Slice blob() = 0;
  // Lifetime of the underlying storage until the next call to Next()
  // REQUIRES: Valid()
  virtual const FeatureSet& feature_set() = 0;

  virtual Status status() const = 0;

 private:
  // No copying allowed
  Cursor(const Cursor&);
  void operator=(const Cursor&);
};

// SpatialIndexOptions defines a spatial index that will be built on the data
struct SpatialIndexOptions {
  // Spatial indexes are referenced by names
  std::string name;
  // An area that is indexed. If the element is not intersecting with spatial
  // index's bbox, it will not be inserted into the index
  BoundingBox<double> bbox;
  // tile_bits control the granularity of the spatial index. Each dimension of
  // the bbox will be split into (1 << tile_bits) tiles, so there will be a
  // total of (1 << tile_bits)^2 tiles. It is recommended to configure a size of
  // each  tile to be approximately the size of the query on that spatial index
  uint32_t tile_bits;
  SpatialIndexOptions() {}
  SpatialIndexOptions(const std::string& _name,
                      const BoundingBox<double>& _bbox, uint32_t _tile_bits)
      : name(_name), bbox(_bbox), tile_bits(_tile_bits) {}
};

class SpatialDB : public StackableDB {
 public:
  // Creates the SpatialDB with specified list of indexes.
  // REQUIRED: db doesn't exist
  static Status Create(const SpatialDBOptions& options, const std::string& name,
                       const std::vector<SpatialIndexOptions>& spatial_indexes);

  // Open the existing SpatialDB.  The resulting db object will be returned
  // through db parameter.
  // REQUIRED: db was created using SpatialDB::Create
  static Status Open(const SpatialDBOptions& options, const std::string& name,
                     SpatialDB** db, bool read_only = false);

  explicit SpatialDB(DB* db) : StackableDB(db) {}

  // Insert the element into the DB. Element will be inserted into specified
  // spatial_indexes, based on specified bbox.
  // REQUIRES: spatial_indexes.size() > 0
  virtual Status Insert(const WriteOptions& write_options,
                        const BoundingBox<double>& bbox, const Slice& blob,
                        const FeatureSet& feature_set,
                        const std::vector<std::string>& spatial_indexes) = 0;

  // Calling Compact() after inserting a bunch of elements should speed up
  // reading. This is especially useful if you use SpatialDBOptions::bulk_load
  // Num threads determines how many threads we'll use for compactions. Setting
  // this to bigger number will use more IO and CPU, but finish faster
  virtual Status Compact(int num_threads = 1) = 0;

  // Query the specified spatial_index. Query will return all elements that
  // intersect bbox, but it may also return some extra elements.
  virtual Cursor* Query(const ReadOptions& read_options,
                        const BoundingBox<double>& bbox,
                        const std::string& spatial_index) = 0;
};

}  // namespace spatial
}  // namespace rocksdb
#endif  // ROCKSDB_LITE
#line 1 "/home/evan/source/rocksdb/include/rocksdb/utilities/table_properties_collectors.h"
//  Copyright (c) 2015, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.

#ifndef ROCKSDB_LITE
#include <memory>


namespace rocksdb {

// Creates a factory of a table property collector that marks a SST
// file as need-compaction when it observe at least "D" deletion
// entries in any "N" consecutive entires.
//
// @param sliding_window_size "N". Note that this number will be
//     round up to the smallest multiple of 128 that is no less
//     than the specified size.
// @param deletion_trigger "D".  Note that even when "N" is changed,
//     the specified number for "D" will not be changed.
extern std::shared_ptr<TablePropertiesCollectorFactory>
    NewCompactOnDeletionCollectorFactory(
        size_t sliding_window_size,
        size_t deletion_trigger);
}  // namespace rocksdb

#endif  // !ROCKSDB_LITE
#line 1 "/home/evan/source/rocksdb/include/rocksdb/utilities/transaction.h"
// Copyright (c) 2015, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.


#ifndef ROCKSDB_LITE

#include <string>
#include <vector>


namespace rocksdb {

class Iterator;
class TransactionDB;
class WriteBatchWithIndex;

// Provides BEGIN/COMMIT/ROLLBACK transactions.
//
// To use transactions, you must first create either an OptimisticTransactionDB
// or a TransactionDB.  See examples/[optimistic_]transaction_example.cc for
// more information.
//
// To create a transaction, use [Optimistic]TransactionDB::BeginTransaction().
//
// It is up to the caller to synchronize access to this object.
//
// See examples/transaction_example.cc for some simple examples.
//
// TODO(agiardullo): Not yet implemented
//  -PerfContext statistics
//  -Support for using Transactions with DBWithTTL
class Transaction {
 public:
  virtual ~Transaction() {}

  // If a transaction has a snapshot set, the transaction will ensure that
  // any keys successfully written(or fetched via GetForUpdate()) have not
  // been modified outside of this transaction since the time the snapshot was
  // set.
  // If a snapshot has not been set, the transaction guarantees that keys have
  // not been modified since the time each key was first written (or fetched via
  // GetForUpdate()).
  //
  // Using SetSnapshot() will provide stricter isolation guarantees at the
  // expense of potentially more transaction failures due to conflicts with
  // other writes.
  //
  // Calling SetSnapshot() has no effect on keys written before this function
  // has been called.
  //
  // SetSnapshot() may be called multiple times if you would like to change
  // the snapshot used for different operations in this transaction.
  //
  // Calling SetSnapshot will not affect the version of Data returned by Get()
  // methods.  See Transaction::Get() for more details.
  virtual void SetSnapshot() = 0;

  // Returns the Snapshot created by the last call to SetSnapshot().
  //
  // REQUIRED: The returned Snapshot is only valid up until the next time
  // SetSnapshot() is called or the Transaction is deleted.
  virtual const Snapshot* GetSnapshot() const = 0;

  // Write all batched keys to the db atomically.
  //
  // Returns OK on success.
  //
  // May return any error status that could be returned by DB:Write().
  //
  // If this transaction was created by an OptimisticTransactionDB(),
  // Status::Busy() may be returned if the transaction could not guarantee
  // that there are no write conflicts.  Status::TryAgain() may be returned
  // if the memtable history size is not large enough
  //  (See max_write_buffer_number_to_maintain).
  //
  // If this transaction was created by a TransactionDB(), Status::Expired()
  // may be returned if this transaction has lived for longer than
  // TransactionOptions.expiration.
  virtual Status Commit() = 0;

  // Discard all batched writes in this transaction.
  virtual void Rollback() = 0;

  // Records the state of the transaction for future calls to
  // RollbackToSavePoint().  May be called multiple times to set multiple save
  // points.
  virtual void SetSavePoint() = 0;

  // Undo all operations in this transaction (Put, Merge, Delete, PutLogData)
  // since the most recent call to SetSavePoint() and removes the most recent
  // SetSavePoint().
  // If there is no previous call to SetSavePoint(), returns Status::NotFound()
  virtual Status RollbackToSavePoint() = 0;

  // This function is similar to DB::Get() except it will also read pending
  // changes in this transaction.  Currently, this function will return
  // Status::MergeInProgress if the most recent write to the queried key in
  // this batch is a Merge.
  //
  // If read_options.snapshot is not set, the current version of the key will
  // be read.  Calling SetSnapshot() does not affect the version of the data
  // returned.
  //
  // Note that setting read_options.snapshot will affect what is read from the
  // DB but will NOT change which keys are read from this transaction (the keys
  // in this transaction do not yet belong to any snapshot and will be fetched
  // regardless).
  virtual Status Get(const ReadOptions& options,
                     ColumnFamilyHandle* column_family, const Slice& key,
                     std::string* value) = 0;

  virtual Status Get(const ReadOptions& options, const Slice& key,
                     std::string* value) = 0;

  virtual std::vector<Status> MultiGet(
      const ReadOptions& options,
      const std::vector<ColumnFamilyHandle*>& column_family,
      const std::vector<Slice>& keys, std::vector<std::string>* values) = 0;

  virtual std::vector<Status> MultiGet(const ReadOptions& options,
                                       const std::vector<Slice>& keys,
                                       std::vector<std::string>* values) = 0;

  // Read this key and ensure that this transaction will only
  // be able to be committed if this key is not written outside this
  // transaction after it has first been read (or after the snapshot if a
  // snapshot is set in this transaction).  The transaction behavior is the
  // same regardless of whether the key exists or not.
  //
  // Note: Currently, this function will return Status::MergeInProgress
  // if the most recent write to the queried key in this batch is a Merge.
  //
  // The values returned by this function are similar to Transaction::Get().
  // If value==nullptr, then this function will not read any data, but will
  // still ensure that this key cannot be written to by outside of this
  // transaction.
  //
  // If this transaction was created by an OptimisticTransaction, GetForUpdate()
  // could cause commit() to fail.  Otherwise, it could return any error
  // that could be returned by DB::Get().
  //
  // If this transaction was created by a TransactionDB, it can return
  // Status::OK() on success,
  // Status::Busy() if there is a write conflict,
  // Status::TimedOut() if a lock could not be acquired,
  // Status::TryAgain() if the memtable history size is not large enough
  //  (See max_write_buffer_number_to_maintain)
  // Status::MergeInProgress() if merge operations cannot be resolved.
  // or other errors if this key could not be read.
  virtual Status GetForUpdate(const ReadOptions& options,
                              ColumnFamilyHandle* column_family,
                              const Slice& key, std::string* value) = 0;

  virtual Status GetForUpdate(const ReadOptions& options, const Slice& key,
                              std::string* value) = 0;

  virtual std::vector<Status> MultiGetForUpdate(
      const ReadOptions& options,
      const std::vector<ColumnFamilyHandle*>& column_family,
      const std::vector<Slice>& keys, std::vector<std::string>* values) = 0;

  virtual std::vector<Status> MultiGetForUpdate(
      const ReadOptions& options, const std::vector<Slice>& keys,
      std::vector<std::string>* values) = 0;

  // Returns an iterator that will iterate on all keys in the default
  // column family including both keys in the DB and uncommitted keys in this
  // transaction.
  //
  // Setting read_options.snapshot will affect what is read from the
  // DB but will NOT change which keys are read from this transaction (the keys
  // in this transaction do not yet belong to any snapshot and will be fetched
  // regardless).
  //
  // Caller is reponsible for deleting the returned Iterator.
  //
  // The returned iterator is only valid until Commit(), Rollback(), or
  // RollbackToSavePoint() is called.
  // NOTE: Transaction::Put/Merge/Delete will currently invalidate this iterator
  // until
  // the following issue is fixed:
  // https://github.com/facebook/rocksdb/issues/616
  virtual Iterator* GetIterator(const ReadOptions& read_options) = 0;

  virtual Iterator* GetIterator(const ReadOptions& read_options,
                                ColumnFamilyHandle* column_family) = 0;

  // Put, Merge, Delete, and SingleDelete behave similarly to the corresponding
  // functions in WriteBatch, but will also do conflict checking on the
  // keys being written.
  //
  // If this Transaction was created on an OptimisticTransactionDB, these
  // functions should always return Status::OK().
  //
  // If this Transaction was created on a TransactionDB, the status returned
  // can be:
  // Status::OK() on success,
  // Status::Busy() if there is a write conflict,
  // Status::TimedOut() if a lock could not be acquired,
  // Status::TryAgain() if the memtable history size is not large enough
  //  (See max_write_buffer_number_to_maintain)
  // or other errors on unexpected failures.
  virtual Status Put(ColumnFamilyHandle* column_family, const Slice& key,
                     const Slice& value) = 0;
  virtual Status Put(const Slice& key, const Slice& value) = 0;
  virtual Status Put(ColumnFamilyHandle* column_family, const SliceParts& key,
                     const SliceParts& value) = 0;
  virtual Status Put(const SliceParts& key, const SliceParts& value) = 0;

  virtual Status Merge(ColumnFamilyHandle* column_family, const Slice& key,
                       const Slice& value) = 0;
  virtual Status Merge(const Slice& key, const Slice& value) = 0;

  virtual Status Delete(ColumnFamilyHandle* column_family,
                        const Slice& key) = 0;
  virtual Status Delete(const Slice& key) = 0;
  virtual Status Delete(ColumnFamilyHandle* column_family,
                        const SliceParts& key) = 0;
  virtual Status Delete(const SliceParts& key) = 0;

  virtual Status SingleDelete(ColumnFamilyHandle* column_family,
                              const Slice& key) = 0;
  virtual Status SingleDelete(const Slice& key) = 0;
  virtual Status SingleDelete(ColumnFamilyHandle* column_family,
                              const SliceParts& key) = 0;
  virtual Status SingleDelete(const SliceParts& key) = 0;

  // PutUntracked() will write a Put to the batch of operations to be committed
  // in this transaction.  This write will only happen if this transaction
  // gets committed successfully.  But unlike Transaction::Put(),
  // no conflict checking will be done for this key.
  //
  // If this Transaction was created on a TransactionDB, this function will
  // still acquire locks necessary to make sure this write doesn't cause
  // conflicts in other transactions and may return Status::Busy().
  virtual Status PutUntracked(ColumnFamilyHandle* column_family,
                              const Slice& key, const Slice& value) = 0;
  virtual Status PutUntracked(const Slice& key, const Slice& value) = 0;
  virtual Status PutUntracked(ColumnFamilyHandle* column_family,
                              const SliceParts& key,
                              const SliceParts& value) = 0;
  virtual Status PutUntracked(const SliceParts& key,
                              const SliceParts& value) = 0;

  virtual Status MergeUntracked(ColumnFamilyHandle* column_family,
                                const Slice& key, const Slice& value) = 0;
  virtual Status MergeUntracked(const Slice& key, const Slice& value) = 0;

  virtual Status DeleteUntracked(ColumnFamilyHandle* column_family,
                                 const Slice& key) = 0;

  virtual Status DeleteUntracked(const Slice& key) = 0;
  virtual Status DeleteUntracked(ColumnFamilyHandle* column_family,
                                 const SliceParts& key) = 0;
  virtual Status DeleteUntracked(const SliceParts& key) = 0;

  // Similar to WriteBatch::PutLogData
  virtual void PutLogData(const Slice& blob) = 0;

  // Returns the number of distinct Keys being tracked by this transaction.
  // If this transaction was created by a TransactinDB, this is the number of
  // keys that are currently locked by this transaction.
  // If this transaction was created by an OptimisticTransactionDB, this is the
  // number of keys that need to be checked for conflicts at commit time.
  virtual uint64_t GetNumKeys() const = 0;

  // Returns the number of Puts/Deletes/Merges that have been applied to this
  // transaction so far.
  virtual uint64_t GetNumPuts() const = 0;
  virtual uint64_t GetNumDeletes() const = 0;
  virtual uint64_t GetNumMerges() const = 0;

  // Returns the elapsed time in milliseconds since this Transaction began.
  virtual uint64_t GetElapsedTime() const = 0;

  // Fetch the underlying write batch that contains all pending changes to be
  // committed.
  //
  // Note:  You should not write or delete anything from the batch directly and
  // should only use the the functions in the Transaction class to
  // write to this transaction.
  virtual WriteBatchWithIndex* GetWriteBatch() = 0;

  // Change the value of TransactionOptions.lock_timeout (in milliseconds) for
  // this transaction.
  // Has no effect on OptimisticTransactions.
  virtual void SetLockTimeout(int64_t timeout) = 0;

 protected:
  explicit Transaction(const TransactionDB* db) {}
  Transaction() {}

 private:
  // No copying allowed
  Transaction(const Transaction&);
  void operator=(const Transaction&);
};

}  // namespace rocksdb

#endif  // ROCKSDB_LITE
#line 1 "/home/evan/source/rocksdb/include/rocksdb/utilities/optimistic_transaction_db.h"
//  Copyright (c) 2015, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.

#ifndef ROCKSDB_LITE

#include <string>
#include <vector>


namespace rocksdb {

class Transaction;

// Database with Transaction support.
//
// See optimistic_transaction.h and examples/transaction_example.cc

// Options to use when starting an Optimistic Transaction
struct OptimisticTransactionOptions {
  // Setting set_snapshot=true is the same as calling SetSnapshot().
  bool set_snapshot = false;

  // Should be set if the DB has a non-default comparator.
  // See comment in WriteBatchWithIndex constructor.
  const Comparator* cmp = BytewiseComparator();
};

class OptimisticTransactionDB {
 public:
  // Open an OptimisticTransactionDB similar to DB::Open().
  static Status Open(const Options& options, const std::string& dbname,
                     OptimisticTransactionDB** dbptr);

  static Status Open(const DBOptions& db_options, const std::string& dbname,
                     const std::vector<ColumnFamilyDescriptor>& column_families,
                     std::vector<ColumnFamilyHandle*>* handles,
                     OptimisticTransactionDB** dbptr);

  virtual ~OptimisticTransactionDB() {}

  // Starts a new Transaction.  Passing set_snapshot=true has the same effect
  // as calling SetSnapshot().
  //
  // Caller should delete the returned transaction after calling
  // Commit() or Rollback().
  virtual Transaction* BeginTransaction(
      const WriteOptions& write_options,
      const OptimisticTransactionOptions&
          txn_options = OptimisticTransactionOptions()) = 0;

  // Return the underlying Database that was opened
  virtual DB* GetBaseDB() = 0;

 protected:
  // To Create an OptimisticTransactionDB, call Open()
  explicit OptimisticTransactionDB(DB* db) {}
  OptimisticTransactionDB() {}

 private:
  // No copying allowed
  OptimisticTransactionDB(const OptimisticTransactionDB&);
  void operator=(const OptimisticTransactionDB&);
};

}  // namespace rocksdb

#endif  // ROCKSDB_LITE
#line 1 "/home/evan/source/rocksdb/include/rocksdb/utilities/write_batch_with_index.h"
// Copyright (c) 2013, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// A WriteBatchWithIndex with a binary searchable index built for all the keys
// inserted.

#ifndef ROCKSDB_LITE

#include <string>


namespace rocksdb {

class ColumnFamilyHandle;
class Comparator;
class DB;
struct ReadOptions;
struct DBOptions;

enum WriteType {
  kPutRecord,
  kMergeRecord,
  kDeleteRecord,
  kSingleDeleteRecord,
  kLogDataRecord
};

// an entry for Put, Merge, Delete, or SingleDelete entry for write batches.
// Used in WBWIIterator.
struct WriteEntry {
  WriteType type;
  Slice key;
  Slice value;
};

// Iterator of one column family out of a WriteBatchWithIndex.
class WBWIIterator {
 public:
  virtual ~WBWIIterator() {}

  virtual bool Valid() const = 0;

  virtual void SeekToFirst() = 0;

  virtual void SeekToLast() = 0;

  virtual void Seek(const Slice& key) = 0;

  virtual void Next() = 0;

  virtual void Prev() = 0;

  // the return WriteEntry is only valid until the next mutation of
  // WriteBatchWithIndex
  virtual WriteEntry Entry() const = 0;

  virtual Status status() const = 0;
};

// A WriteBatchWithIndex with a binary searchable index built for all the keys
// inserted.
// In Put(), Merge() Delete(), or SingleDelete(), the same function of the
// wrapped will be called. At the same time, indexes will be built.
// By calling GetWriteBatch(), a user will get the WriteBatch for the data
// they inserted, which can be used for DB::Write().
// A user can call NewIterator() to create an iterator.
class WriteBatchWithIndex : public WriteBatchBase {
 public:
  // backup_index_comparator: the backup comparator used to compare keys
  // within the same column family, if column family is not given in the
  // interface, or we can't find a column family from the column family handle
  // passed in, backup_index_comparator will be used for the column family.
  // reserved_bytes: reserved bytes in underlying WriteBatch
  // overwrite_key: if true, overwrite the key in the index when inserting
  //                the same key as previously, so iterator will never
  //                show two entries with the same key.
  explicit WriteBatchWithIndex(
      const Comparator* backup_index_comparator = BytewiseComparator(),
      size_t reserved_bytes = 0, bool overwrite_key = false);
  virtual ~WriteBatchWithIndex();

  using WriteBatchBase::Put;
  void Put(ColumnFamilyHandle* column_family, const Slice& key,
           const Slice& value) override;

  void Put(const Slice& key, const Slice& value) override;

  using WriteBatchBase::Merge;
  void Merge(ColumnFamilyHandle* column_family, const Slice& key,
             const Slice& value) override;

  void Merge(const Slice& key, const Slice& value) override;

  using WriteBatchBase::Delete;
  void Delete(ColumnFamilyHandle* column_family, const Slice& key) override;
  void Delete(const Slice& key) override;

  using WriteBatchBase::SingleDelete;
  void SingleDelete(ColumnFamilyHandle* column_family,
                    const Slice& key) override;
  void SingleDelete(const Slice& key) override;

  using WriteBatchBase::PutLogData;
  void PutLogData(const Slice& blob) override;

  using WriteBatchBase::Clear;
  void Clear() override;

  using WriteBatchBase::GetWriteBatch;
  WriteBatch* GetWriteBatch() override;

  // Create an iterator of a column family. User can call iterator.Seek() to
  // search to the next entry of or after a key. Keys will be iterated in the
  // order given by index_comparator. For multiple updates on the same key,
  // each update will be returned as a separate entry, in the order of update
  // time.
  //
  // The returned iterator should be deleted by the caller.
  WBWIIterator* NewIterator(ColumnFamilyHandle* column_family);
  // Create an iterator of the default column family.
  WBWIIterator* NewIterator();

  // Will create a new Iterator that will use WBWIIterator as a delta and
  // base_iterator as base.
  //
  // This function is only supported if the WriteBatchWithIndex was
  // constructed with overwrite_key=true.
  //
  // The returned iterator should be deleted by the caller.
  // The base_iterator is now 'owned' by the returned iterator. Deleting the
  // returned iterator will also delete the base_iterator.
  Iterator* NewIteratorWithBase(ColumnFamilyHandle* column_family,
                                Iterator* base_iterator);
  // default column family
  Iterator* NewIteratorWithBase(Iterator* base_iterator);

  // Similar to DB::Get() but will only read the key from this batch.
  // If the batch does not have enough data to resolve Merge operations,
  // MergeInProgress status may be returned.
  Status GetFromBatch(ColumnFamilyHandle* column_family,
                      const DBOptions& options, const Slice& key,
                      std::string* value);

  // Similar to previous function but does not require a column_family.
  // Note:  An InvalidArgument status will be returned if there are any Merge
  // operators for this key.  Use previous method instead.
  Status GetFromBatch(const DBOptions& options, const Slice& key,
                      std::string* value) {
    return GetFromBatch(nullptr, options, key, value);
  }

  // Similar to DB::Get() but will also read writes from this batch.
  //
  // This function will query both this batch and the DB and then merge
  // the results using the DB's merge operator (if the batch contains any
  // merge requests).
  //
  // Setting read_options.snapshot will affect what is read from the DB
  // but will NOT change which keys are read from the batch (the keys in
  // this batch do not yet belong to any snapshot and will be fetched
  // regardless).
  Status GetFromBatchAndDB(DB* db, const ReadOptions& read_options,
                           const Slice& key, std::string* value);
  Status GetFromBatchAndDB(DB* db, const ReadOptions& read_options,
                           ColumnFamilyHandle* column_family, const Slice& key,
                           std::string* value);

  // Records the state of the batch for future calls to RollbackToSavePoint().
  // May be called multiple times to set multiple save points.
  void SetSavePoint() override;

  // Remove all entries in this batch (Put, Merge, Delete, SingleDelete,
  // PutLogData) since the most recent call to SetSavePoint() and removes the
  // most recent save point.
  // If there is no previous call to SetSavePoint(), behaves the same as
  // Clear().
  //
  // Calling RollbackToSavePoint invalidates any open iterators on this batch.
  //
  // Returns Status::OK() on success,
  //         Status::NotFound() if no previous call to SetSavePoint(),
  //         or other Status on corruption.
  Status RollbackToSavePoint() override;

 private:
  struct Rep;
  Rep* rep;
};

}  // namespace rocksdb

#endif  // !ROCKSDB_LITE
#line 1 "/home/evan/source/rocksdb/include/rocksdb/utilities/transaction_db.h"
//  Copyright (c) 2015, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.

#ifndef ROCKSDB_LITE

#include <string>
#include <vector>


// Database with Transaction support.
//
// See transaction.h and examples/transaction_example.cc

namespace rocksdb {

class TransactionDBMutexFactory;

struct TransactionDBOptions {
  // Specifies the maximum number of keys that can be locked at the same time
  // per column family.
  // If the number of locked keys is greater than max_num_locks, transaction
  // writes (or GetForUpdate) will return an error.
  // If this value is not positive, no limit will be enforced.
  int64_t max_num_locks = -1;

  // Increasing this value will increase the concurrency by dividing the lock
  // table (per column family) into more sub-tables, each with their own
  // separate
  // mutex.
  size_t num_stripes = 16;

  // If positive, specifies the default wait timeout in milliseconds when
  // a transaction attempts to lock a key if not specified by
  // TransactionOptions::lock_timeout.
  //
  // If 0, no waiting is done if a lock cannot instantly be acquired.
  // If negative, there is no timeout.  Not using a timeout is not recommended
  // as it can lead to deadlocks.  Currently, there is no deadlock-detection to
  // recover
  // from a deadlock.
  int64_t transaction_lock_timeout = 1000;  // 1 second

  // If positive, specifies the wait timeout in milliseconds when writing a key
  // OUTSIDE of a transaction (ie by calling DB::Put(),Merge(),Delete(),Write()
  // directly).
  // If 0, no waiting is done if a lock cannot instantly be acquired.
  // If negative, there is no timeout and will block indefinitely when acquiring
  // a lock.
  //
  // Not using a a timeout can lead to deadlocks.  Currently, there
  // is no deadlock-detection to recover from a deadlock.  While DB writes
  // cannot deadlock with other DB writes, they can deadlock with a transaction.
  // A negative timeout should only be used if all transactions have an small
  // expiration set.
  int64_t default_lock_timeout = 1000;  // 1 second

  // If set, the TransactionDB will use this implemenation of a mutex and
  // condition variable for all transaction locking instead of the default
  // mutex/condvar implementation.
  std::shared_ptr<TransactionDBMutexFactory> custom_mutex_factory;
};

struct TransactionOptions {
  // Setting set_snapshot=true is the same as calling
  // Transaction::SetSnapshot().
  bool set_snapshot = false;


  // TODO(agiardullo): TransactionDB does not yet support comparators that allow
  // two non-equal keys to be equivalent.  Ie, cmp->Compare(a,b) should only
  // return 0 if
  // a.compare(b) returns 0.


  // If positive, specifies the wait timeout in milliseconds when
  // a transaction attempts to lock a key.
  //
  // If 0, no waiting is done if a lock cannot instantly be acquired.
  // If negative, TransactionDBOptions::transaction_lock_timeout will be used.
  int64_t lock_timeout = -1;

  // Expiration duration in milliseconds.  If non-negative, transactions that
  // last longer than this many milliseconds will fail to commit.  If not set,
  // a forgotten transaction that is never committed, rolled back, or deleted
  // will never relinquish any locks it holds.  This could prevent keys from
  // being
  // written by other writers.
  //
  // TODO(agiardullo):  Improve performance of checking expiration time.
  int64_t expiration = -1;
};

class TransactionDB : public StackableDB {
 public:
  // Open a TransactionDB similar to DB::Open().
  static Status Open(const Options& options,
                     const TransactionDBOptions& txn_db_options,
                     const std::string& dbname, TransactionDB** dbptr);

  static Status Open(const DBOptions& db_options,
                     const TransactionDBOptions& txn_db_options,
                     const std::string& dbname,
                     const std::vector<ColumnFamilyDescriptor>& column_families,
                     std::vector<ColumnFamilyHandle*>* handles,
                     TransactionDB** dbptr);

  virtual ~TransactionDB() {}

  // Starts a new Transaction.  Passing set_snapshot=true has the same effect
  // as calling Transaction::SetSnapshot().
  //
  // Caller should delete the returned transaction after calling
  // Transaction::Commit() or Transaction::Rollback().
  virtual Transaction* BeginTransaction(
      const WriteOptions& write_options,
      const TransactionOptions& txn_options = TransactionOptions()) = 0;

 protected:
  // To Create an TransactionDB, call Open()
  explicit TransactionDB(DB* db) : StackableDB(db) {}

 private:
  // No copying allowed
  TransactionDB(const TransactionDB&);
  void operator=(const TransactionDB&);
};

}  // namespace rocksdb

#endif  // ROCKSDB_LITE
#line 1 "/home/evan/source/rocksdb/include/rocksdb/utilities/transaction_db_mutex.h"
//  Copyright (c) 2015, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.

#ifndef ROCKSDB_LITE

#include <memory>


namespace rocksdb {

// TransactionDBMutex and TransactionDBCondVar APIs allows applications to
// implement custom mutexes and condition variables to be used by a
// TransactionDB when locking keys.
//
// To open a TransactionDB with a custom TransactionDBMutexFactory, set
// TransactionDBOptions.custom_mutex_factory.

class TransactionDBMutex {
 public:
  virtual ~TransactionDBMutex() {}

  // Attempt to acquire lock.  Return OK on success, or other Status on failure.
  // If returned status is OK, TransactionDB will eventually call UnLock().
  virtual Status Lock() = 0;

  // Attempt to acquire lock.  If timeout is non-negative, operation should be
  // failed after this many microseconds.
  // Returns OK on success,
  //         TimedOut if timed out,
  //         or other Status on failure.
  // If returned status is OK, TransactionDB will eventually call UnLock().
  virtual Status TryLockFor(int64_t timeout_time) = 0;

  // Unlock Mutex that was successfully locked by Lock() or TryLockUntil()
  virtual void UnLock() = 0;
};

class TransactionDBCondVar {
 public:
  virtual ~TransactionDBCondVar() {}

  // Block current thread until condition variable is notified by a call to
  // Notify() or NotifyAll().  Wait() will be called with mutex locked.
  // Returns OK if notified.
  // Returns non-OK if TransactionDB should stop waiting and fail the operation.
  // May return OK spuriously even if not notified.
  virtual Status Wait(std::shared_ptr<TransactionDBMutex> mutex) = 0;

  // Block current thread until condition variable is notified by a call to
  // Notify() or NotifyAll(), or if the timeout is reached.
  // Wait() will be called with mutex locked.
  //
  // If timeout is non-negative, operation should be failed after this many
  // microseconds.
  // If implementing a custom version of this class, the implementation may
  // choose to ignore the timeout.
  //
  // Returns OK if notified.
  // Returns TimedOut if timeout is reached.
  // Returns other status if TransactionDB should otherwis stop waiting and
  //  fail the operation.
  // May return OK spuriously even if not notified.
  virtual Status WaitFor(std::shared_ptr<TransactionDBMutex> mutex,
                         int64_t timeout_time) = 0;

  // If any threads are waiting on *this, unblock at least one of the
  // waiting threads.
  virtual void Notify() = 0;

  // Unblocks all threads waiting on *this.
  virtual void NotifyAll() = 0;
};

// Factory class that can allocate mutexes and condition variables.
class TransactionDBMutexFactory {
 public:
  // Create a TransactionDBMutex object.
  virtual std::shared_ptr<TransactionDBMutex> AllocateMutex() = 0;

  // Create a TransactionDBCondVar object.
  virtual std::shared_ptr<TransactionDBCondVar> AllocateCondVar() = 0;

  virtual ~TransactionDBMutexFactory() {}
};

}  // namespace rocksdb

#endif  // ROCKSDB_LITE
#line 1 "/home/evan/source/rocksdb/include/rocksdb/utilities/utility_db.h"
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef ROCKSDB_LITE
#include <vector>
#include <string>

#line 1 "/home/evan/source/rocksdb/include/rocksdb/utilities/db_ttl.h"
//  Copyright (c) 2013, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.

#ifndef ROCKSDB_LITE

#include <string>
#include <vector>


namespace rocksdb {

// Database with TTL support.
//
// USE-CASES:
// This API should be used to open the db when key-values inserted are
//  meant to be removed from the db in a non-strict 'ttl' amount of time
//  Therefore, this guarantees that key-values inserted will remain in the
//  db for >= ttl amount of time and the db will make efforts to remove the
//  key-values as soon as possible after ttl seconds of their insertion.
//
// BEHAVIOUR:
// TTL is accepted in seconds
// (int32_t)Timestamp(creation) is suffixed to values in Put internally
// Expired TTL values deleted in compaction only:(Timestamp+ttl<time_now)
// Get/Iterator may return expired entries(compaction not run on them yet)
// Different TTL may be used during different Opens
// Example: Open1 at t=0 with ttl=4 and insert k1,k2, close at t=2
//          Open2 at t=3 with ttl=5. Now k1,k2 should be deleted at t>=5
// read_only=true opens in the usual read-only mode. Compactions will not be
//  triggered(neither manual nor automatic), so no expired entries removed
//
// CONSTRAINTS:
// Not specifying/passing or non-positive TTL behaves like TTL = infinity
//
// !!!WARNING!!!:
// Calling DB::Open directly to re-open a db created by this API will get
//  corrupt values(timestamp suffixed) and no ttl effect will be there
//  during the second Open, so use this API consistently to open the db
// Be careful when passing ttl with a small positive value because the
//  whole database may be deleted in a small amount of time

class DBWithTTL : public StackableDB {
 public:
  virtual Status CreateColumnFamilyWithTtl(
      const ColumnFamilyOptions& options, const std::string& column_family_name,
      ColumnFamilyHandle** handle, int ttl) = 0;

  static Status Open(const Options& options, const std::string& dbname,
                     DBWithTTL** dbptr, int32_t ttl = 0,
                     bool read_only = false);

  static Status Open(const DBOptions& db_options, const std::string& dbname,
                     const std::vector<ColumnFamilyDescriptor>& column_families,
                     std::vector<ColumnFamilyHandle*>* handles,
                     DBWithTTL** dbptr, std::vector<int32_t> ttls,
                     bool read_only = false);

 protected:
  explicit DBWithTTL(DB* db) : StackableDB(db) {}
};

}  // namespace rocksdb
#endif  // ROCKSDB_LITE
#line 11 "/home/evan/source/rocksdb/include/rocksdb/utilities/utility_db.h"

namespace rocksdb {

// Please don't use this class. It's deprecated
class UtilityDB {
 public:
  // This function is here only for backwards compatibility. Please use the
  // functions defined in DBWithTTl (rocksdb/utilities/db_ttl.h)
  // (deprecated)
#if defined(__GNUC__) || defined(__clang__)
  __attribute__((deprecated))
#elif _WIN32
   __declspec(deprecated)
#endif
    static Status OpenTtlDB(const Options& options,
                                                      const std::string& name,
                                                      StackableDB** dbptr,
                                                      int32_t ttl = 0,
                                                      bool read_only = false);
};

} //  namespace rocksdb
#endif  // ROCKSDB_LITE
