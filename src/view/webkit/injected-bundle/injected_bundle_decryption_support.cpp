/*
 * Copyright (c) 2012 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/*
 * @file    injected_bundle_decryption_support.cpp
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 * @version 1.0
 */

#include "injected_bundle_decryption_support.h"

#include <memory>
#include <set>
#include <string>
#include <sys/stat.h>
#include <ss_manager.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include <dpl/assert.h>
#include <dpl/string.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <dpl/utils/mime_type_utils.h>

#include <dpl/log/secure_log.h>
#include <pkgmgr-info.h>

namespace InjectedBundle {
namespace {
const char * const SCHEME_FILE_SLASH = "file://";
const char * const DATA_STRING = "data:";
const char * const BASE64_STRING = ";base64,";
const char QUESTION_MARK = '?';
const char ASTERISK_MARK = '#';
}

//Implementation class
class DecryptionSupportImplementation
{
  private:
    bool m_initialized;

    WrtDB::TizenAppId m_appId;
    WrtDB::EncryptedFileList m_encryptedFiles;
    bool m_isEncrypted;
    std::set<WrtDB::EncryptedFileInfo>::iterator m_targetIt;
    bool m_isPreload;


    std::string getFilePath(const std::string& url)
    {
        std::string filePath = url;

        size_t pos = filePath.find_first_not_of(SCHEME_FILE_SLASH);
        if (pos != std::string::npos) {
            filePath = filePath.substr(pos - 1);
        }

        pos = filePath.find_first_of(ASTERISK_MARK);
        if (pos != std::string::npos) {
            filePath = filePath.substr(0, pos);
        }

        pos = filePath.find_first_of(QUESTION_MARK);
        if (pos != std::string::npos) {
            filePath = filePath.substr(0, pos);
        }

        return filePath;
    }

    int ssmDecrypt(const char* inBuf, int inSize, char** outBuf, int* outSize)
    {
        *outSize = ssa_decrypt_web_application(inBuf, inSize, outBuf, m_isPreload);
	return *outSize;
    }

    std::string doDecrypt(std::string filePath, int size)
    {
        struct stat buf;
        if (0 == stat(filePath.c_str(), &buf)) {
            const std::size_t fileSize = buf.st_size;
            std::unique_ptr<unsigned char[]> inChunk;

            FILE* fp = fopen(filePath.c_str(), "rb");
            if (NULL == fp) {
                _E("Couldnot open file : %s", filePath.c_str());
                return std::string();
            }

            std::unique_ptr<unsigned char[]> DecryptedString(new unsigned
                    char[fileSize]);
            std::string pkgid(DPL::ToUTF8String(m_appId));

            int writeCount = 0;
            do {
                unsigned char getDecSize[4];
                memset(getDecSize, 0x00, sizeof(getDecSize));

                size_t readSize = fread(getDecSize, sizeof(unsigned char), sizeof(getDecSize), fp);
                if (0 != readSize) {
                    unsigned int readBufSize = 0;
                    std::istringstream(std::string((char*)getDecSize)) >> readBufSize;
                    if (readBufSize == 0) {
                        _E("Failed to read resource");
                        fclose(fp);
                        return std::string();
                    }
                    inChunk.reset(new unsigned char[readBufSize]);

                    size_t decReadSize = fread(inChunk.get(), sizeof(unsigned char), readBufSize, fp);

                    if (0 != decReadSize) {
                        char *outChunk = NULL;
                        int outSize = 0;
                        if (0 > ssmDecrypt((char*)inChunk.get(), (int)decReadSize, &outChunk, &outSize))
                        {
                            _E("Failed to get decrypted resource");
                            fclose(fp);
                            return std::string();
                        }
                        memcpy(DecryptedString.get() + writeCount, outChunk, outSize);
                        writeCount += outSize;
                    }
                }
            } while( 0 == std::feof(fp));
            fclose(fp);
            memset(DecryptedString.get() + size, '\n', fileSize - size);

            BIO *bmem, *b64;
            BUF_MEM *bptr;

            b64 = BIO_new(BIO_f_base64());
            bmem = BIO_new(BIO_s_mem());
            b64 = BIO_push(b64, bmem);
            if (BIO_write(b64, DecryptedString.get(), fileSize) <= 0) {
                _E("No data has been written");
            }
            BIO_flush(b64);
            BIO_get_mem_ptr(b64, &bptr);

            std::string base64Enc((char *)bptr->data, bptr->length - 1);
            BIO_free_all(b64);

            return base64Enc;
        }
        return std::string();
    }

  public:
    DecryptionSupportImplementation() :
        m_initialized(false),
        m_isEncrypted(false),
        m_isPreload(false)
    {
    }

    void initialize(WrtDB::TizenAppId appId)
    {
        _D("called");
        m_initialized = true;

        m_appId = appId;
        WrtDB::WidgetDAOReadOnly dao(m_appId);
        dao.getEncryptedFileList(m_encryptedFiles);
        if (!m_encryptedFiles.empty()) {
          m_isEncrypted = true;
          _D("encrypted application");
        }

        bool isPreload = false;
        bool isUpdate = false;
        pkgmgrinfo_pkginfo_h handle = NULL;
        std::string tzPkgId = DPL::ToUTF8String(dao.getTizenPkgId());

        if (PMINFO_R_OK != pkgmgrinfo_pkginfo_get_pkginfo(tzPkgId.c_str(), &handle)) {
            _E("Can't get package information : %s", tzPkgId.c_str());
            return;
        }
        if (PMINFO_R_OK != pkgmgrinfo_pkginfo_is_preload(handle, &isPreload)) {
            _E("Can't get package information : %s", tzPkgId.c_str());
            return;
        }
        if (PMINFO_R_OK != pkgmgrinfo_pkginfo_is_update(handle, &isUpdate)) {
            _E("Can't get package information : %s", tzPkgId.c_str());
            return;
        }

        if (isPreload && !isUpdate) {
            m_isPreload = true;
            _D("preload application");
        }
    }

    void deinitialize(void)
    {
        _D("called");

        m_encryptedFiles.clear();
        m_targetIt = m_encryptedFiles.end();
        m_isEncrypted = false;
        m_appId = DPL::String(L"");
        m_initialized = false;
    }

    bool isNeedDecryption(std::string url)
    {
        if (!m_initialized) {
            _E("not initialized");
            return false;
        }

        if (0 != strncmp(url.c_str(), SCHEME_FILE_SLASH, strlen(SCHEME_FILE_SLASH))) {
            return false;
        }

        std::set<WrtDB::EncryptedFileInfo>::iterator it;
        WrtDB::EncryptedFileInfo info;
        std::string filePath = getFilePath(url);
        info.fileName = DPL::FromUTF8String(filePath);
        if (m_encryptedFiles.end() != (it = m_encryptedFiles.find(info))) {
            _D(" info file name : %s", DPL::ToUTF8String(it->fileName).c_str());
            _D(" info file size : %d", it->fileSize);
            m_targetIt = it;
            return true;
        }
        return false;
    }

    std::string decryptResource(std::string url)
    {
        if (!m_initialized) {
            _E("not initialized");
            return std::string();
        }

        std::string filePath = getFilePath(url);
        if (filePath != DPL::ToUTF8String(m_targetIt->fileName)) {
            if (!isNeedDecryption(filePath)) {
                return std::string();
            }
        }

        std::string decryptString =
            doDecrypt(DPL::ToUTF8String(m_targetIt->fileName),
                      m_targetIt->fileSize);
        if (!decryptString.empty()) {
            std::string destString = DATA_STRING;

            std::string mimeString =
                DPL::ToUTF8String(
                    MimeTypeUtils::identifyFileMimeType(
                        DPL::FromUTF8String(url)));

            destString += mimeString;
            destString += BASE64_STRING;

            decryptString.insert(0, destString);
        }
        return decryptString;
    }
};

DecryptionSupport::DecryptionSupport() :
    m_impl(new DecryptionSupportImplementation)
{
}

DecryptionSupport::~DecryptionSupport()
{
}

void DecryptionSupport::initialize(WrtDB::TizenAppId appId)
{
    m_impl->initialize(appId);
}

void DecryptionSupport::deinitialize(void)
{
    m_impl->deinitialize();
}

bool DecryptionSupport::isNeedDecryption(std::string url)
{
    return m_impl->isNeedDecryption(url);
}

std::string DecryptionSupport::decryptResource(std::string url)
{
    return m_impl->decryptResource(url);
}
}  // namespace InjectedBundle
