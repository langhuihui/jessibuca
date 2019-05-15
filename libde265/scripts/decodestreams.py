#!/usr/bin/python
"""
H.265 video codec.
Copyright (c) 2014 struktur AG, Joachim Bauch <bauch@struktur.de>

This file is part of libde265.

libde265 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

libde265 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libde265.  If not, see <http://www.gnu.org/licenses/>.
"""
import glob
try:
    import multiprocessing
except ImportError:
    multiprocessing = None
import os
import subprocess
import sys
import threading
import time

CPU_COUNT = multiprocessing is not None and multiprocessing.cpu_count() or 2
if CPU_COUNT > 2:
    THREAD_COUNT = CPU_COUNT / 2
else:
    THREAD_COUNT = 2

PROCESS_COUNT = min(4, CPU_COUNT)

# print a status every 10 seconds while waiting for pending tasks
STATUS_INTERVAL = 10

# cancel waiting if no more tasks completed for 120 seconds
CANCEL_TIMEOUT = 120

DEFAULT_ROOT = '/var/lib/libde265-teststreams'

def decode_file(filename, threads=None):
    cmd = ['./dec265/dec265', '-q', '-c']
    if threads and threads > 0:
        cmd.append('-t')
        cmd.append(str(threads))
    cmd.append(filename)
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    try:
        (stdoutdata, stderrdata) = p.communicate()
    except KeyboardInterrupt:
        return (True, filename)

    if p.returncode < 0:
        print '\rERROR: %s failed with signal %d (%r)' % (filename, -p.returncode, stdoutdata)
        return (False, filename)
    elif p.returncode > 0:
        basename = os.path.basename(filename)
        if basename[:3] == 'id:':
            # fuzzing files may be invalid
            print '\rWARNING: %s failed with returncode %d' % (filename, p.returncode)
            return (True, filename)
        else:
            print '\rERROR: %s failed with returncode %d (%r)' % (filename, p.returncode, stdoutdata)
            return (False, filename)
    else:
        print 'OK: %s' % (filename)
        return (True, filename)

class BaseProcessor(object):

    def __init__(self, filenames, threads=None):
        self.filenames = filenames
        self.threads = threads
        self.errors = []

    def process(self, filename):
        ok = decode_file(filename, threads=self.threads)
        return (ok, filename)

    def run(self):
        self.errors = []
        for filename in sorted(self.filenames):
            ok, _ = self.process(filename)
            if not ok:
                self.errors.append(filename)

    def cancel(self):
        pass

if multiprocessing is not None:

    class MultiprocessingProcessor(BaseProcessor):

        def __init__(self, *args, **kw):
            super(MultiprocessingProcessor, self).__init__(*args, **kw)
            self.pool = multiprocessing.Pool(PROCESS_COUNT)
            self.pending_jobs = []
            self.errors = []
            self.lock = threading.RLock()

        def process(self, *args, **kw):
            kw.setdefault('threads', self.threads)
            with self.lock:
                job = self.pool.apply_async(decode_file, args, kw, self._callback)
                self.pending_jobs.append(job)

            return (True, None)

        def _check_pending_jobs(self, timeout=0.1):
            with self.lock:
                self.pending_jobs = [x for x in self.pending_jobs if not x.ready()]

            try:
                job = self.pending_jobs[0]
            except IndexError:
                return

            try:
                result = job.get(timeout=timeout)
            except multiprocessing.TimeoutError:
                # result not available yet
                return
            except:
                self.pending_jobs.pop(0)
                raise
            else:
                self.pending_jobs.pop(0)

        def _callback(self, result):
            ok, filename = result
            if not ok:
                self.errors.append(filename)

        def run(self):
            self.errors = []
            try:
                super(MultiprocessingProcessor, self).run()
            except KeyboardInterrupt:
                self.pool.terminate()
            else:
                self.pool.close()
                now = time.time()
                next_status = now
                cancel_time = now + CANCEL_TIMEOUT
                prev_remaining = len(self.pending_jobs)
                while self.pending_jobs:
                    self._check_pending_jobs()
                    now = time.time()
                    remaining = len(self.pending_jobs)
                    if remaining != prev_remaining:
                        cancel_time = now + CANCEL_TIMEOUT
                        prev_remaining = remaining

                    if now >= cancel_time:
                        print 'Timeout while waiting for pending jobs, cancelling...'
                        self.errors.append('Cancelled due to timeout, %d jobs were still pending' % (remaining))
                        self.pool.terminate()
                        break
                    elif remaining and now >= next_status:
                        print 'Wait for %d pending jobs...' % (remaining)
                        next_status = now + STATUS_INTERVAL

            self.pool.join()

    def cancel(self):
        super(MultiprocessingProcessor, self).cancel()
        self.pool.terminate()
        self.pool.join()

    ProcessorClass = MultiprocessingProcessor
else:
    ProcessorClass = BaseProcessor

def main():
    argv = sys.argv[:]
    try:
        argv.remove('--single-threaded')
    except ValueError:
        threads = THREAD_COUNT
    else:
        threads = None

    if len(argv) > 1:
        root = argv[1]
        if not os.path.isdir(root):
            root = DEFAULT_ROOT
    else:
        root = DEFAULT_ROOT
    
    filenames = glob.glob(os.path.join(root, '*.bin'))
    print 'Processing %d streams in %s' % (len(filenames), root)
    if threads:
        print 'Using %d processes with %s threads each' % (PROCESS_COUNT, threads)
    else:
        print 'Using %d processes' % (PROCESS_COUNT)
    
    processor = ProcessorClass(filenames, threads)
    try:
        processor.run()
    except KeyboardInterrupt:
        processor.cancel()
        print 'Cancelled...'

    if processor.errors:
        print 'Found %d files with errors:' % (len(processor.errors))
        print '\n'.join(sorted(processor.errors))
        sys.exit(1)

if __name__ == '__main__':
    main()
