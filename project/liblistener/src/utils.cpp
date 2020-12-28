#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <iostream>
#include <stdexcept>

#include "Listener.h"

namespace gogo {
    namespace {
// Directory name template must contain XXXXXX.
        constexpr char TEMP_FILE_NAME[] = "gogo.tmp.XXXXXX";
    }

    static FilePath CreateTemporaryDirInDirImpl(const FilePath& base_dir,
                                                const std::string& name_tmpl) {
        FilePath sub_dir = base_dir / name_tmpl;
        const std::string sub_dir_string = sub_dir.native();

        // this should be OK since mkdtemp just replaces characters in place
        char* buffer = const_cast<char*>(sub_dir_string.c_str());
        char* dtemp = mkdtemp(buffer);
        if (!dtemp)
            throw std::runtime_error("mkdtemp failed");

        return FilePath(dtemp);
    }

    FilePath GetTempDir() noexcept {
        const char* tmp = getenv("TMPDIR");
        if (tmp)
            return FilePath(tmp);

        return FilePath("/tmp");
    }

    FilePath CreateNewTempDirectory() {
        return CreateTemporaryDirInDirImpl(GetTempDir(), TEMP_FILE_NAME);
    }

    FilePath CreateTemporaryDirInDir(const FilePath& base_dir,
                                     const std::string& prefix) {
        const std::string mkdtemp_template = prefix + "XXXXXX";
        return CreateTemporaryDirInDirImpl(base_dir, mkdtemp_template);
    }

    bool WriteFileDescriptor(const int fd, const char* data, int size) noexcept {
        // Allow for partial writes.
        ssize_t bytes_written_total = 0;
        for (ssize_t bytes_written_partial = 0; bytes_written_total < size;
             bytes_written_total += bytes_written_partial) {
            bytes_written_partial = HANDLE_EINTR(write(fd, data + bytes_written_total,
                                                       size - bytes_written_total));
            if (bytes_written_partial < 0)
                return false;
        }

        return true;
    }

    int WriteFile(const FilePath& filename, const char* data, int size) noexcept {
        const int mode = 0666;
        const int fd = HANDLE_EINTR(creat(filename.native().c_str(), mode));
        if (fd < 0)
            return -1;

        const int bytes_written = WriteFileDescriptor(fd, data, size) ? size : -1;
        if (close(fd) < 0)
            return -1;
        return bytes_written;
    }

}  // namespace gogo
