//
// Created by satur on 10/22/21.
//
#include <cstdio>
#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>
#include <cerrno>
#include <ctime>

#define TXMP_STRING_TYPE std::string
#define XMP_INCLUDE_XMPFILES 1

#include "XMP.hpp"
#include "XMP.incl_cpp"
#include "logutils.h"

using namespace std;

#if WIN_ENV
#pragma warning ( disable : 4100 )	// ignore unused variable
#pragma warning ( disable : 4127 )	// conditional expression is constant
#pragma warning ( disable : 4505 )	// unreferenced local function has been removed
#pragma warning ( disable : 4996 )	// '...' was declared deprecated
#endif

// -------------------------------------------------------------------------------------------------

FILE *sLogFile = 0;

// -------------------------------------------------------------------------------------------------

static void WriteMinorLabel(FILE *log, const char *title) {
//    for ( size_t i = 0; i < strlen(title); ++i ) fprintf ( log, "-" );
    LOGW("\t--%s :\n", title);
}    // WriteMinorLabel

// -------------------------------------------------------------------------------------------------

static XMP_Status DumpToFile(void *refCon, XMP_StringPtr outStr, XMP_StringLen outLen) {
    XMP_Status status = 0;
    size_t count;
    FILE *outFile = static_cast < FILE * > ( refCon );

    count = fwrite(outStr, 1, outLen, outFile);
    if (count != outLen) status = errno;
    return status;

}    // DumpToFile

// -------------------------------------------------------------------------------------------------

static XMP_Status DumpToString(void *refCon, XMP_StringPtr outStr, XMP_StringLen outLen) {
    std::string *fullStr = static_cast < std::string * > ( refCon );

    fullStr->append(outStr, outLen);
    return 0;

}    // DumpToString

// -------------------------------------------------------------------------------------------------

#define DumpOneFormat(fmt)                                                    \
    format = kXMP_ ## fmt ## File;                                            \
    flags = 0;                                                                \
    ok = SXMPFiles::GetFormatInfo ( format, &flags );                        \
    LOGE("kXMP_" #fmt "File = %.8X, %s, flags = 0x%X\n",        \
              format, (ok ? "smart" : "dumb"), flags );

static void DumpHandlerInfo() {

    WriteMinorLabel(sLogFile, "Dump file format constants and handler flags");

    bool ok;
    XMP_FileFormat format;
    XMP_OptionBits flags;

    DumpOneFormat (PDF);
    DumpOneFormat (PostScript);
    DumpOneFormat (EPS);

    DumpOneFormat (JPEG);
    DumpOneFormat (JPEG2K);
    DumpOneFormat (TIFF);
    DumpOneFormat (GIF);
    DumpOneFormat (PNG);

    DumpOneFormat (MOV);
    DumpOneFormat (AVI);
    DumpOneFormat (CIN);
    DumpOneFormat (WAV);
    DumpOneFormat (MP3);
    DumpOneFormat (SES);
    DumpOneFormat (CEL);
    DumpOneFormat (MPEG);
    DumpOneFormat (MPEG2);
    DumpOneFormat (WMAV);

    DumpOneFormat (HTML);
    DumpOneFormat (XML);
    DumpOneFormat (Text);

    DumpOneFormat (Photoshop);
    DumpOneFormat (Illustrator);
    DumpOneFormat (InDesign);
    DumpOneFormat (AEProject);
    DumpOneFormat (AEFilterPreset);
    DumpOneFormat (EncoreProject);
    DumpOneFormat (PremiereProject);
    DumpOneFormat (PremiereTitle);

    DumpOneFormat (Unknown);

}    // DumpHandlerInfo

// -------------------------------------------------------------------------------------------------

static void
OpenTestFile(const char *fileName, XMP_OptionBits rwMode, SXMPMeta *xmpMeta, SXMPFiles *xmpFile) {
    bool ok;

    XMP_FileFormat format;
    XMP_OptionBits openFlags, handlerFlags;
    XMP_PacketInfo xmpPacket;

    bool isUpdate = ((rwMode & kXMPFiles_OpenForUpdate) != 0);

    static const char *charForms[] = {"UTF-8", "unknown char form", "UTF-16BE", "UTF-16LE",
                                      "UTF-32BE", "UTF-32LE"};

    XMP_OptionBits smartFlags = rwMode | kXMPFiles_OpenUseSmartHandler;
    XMP_OptionBits scanFlags = rwMode | kXMPFiles_OpenUsePacketScanning;

//    ok = xmpFile->OpenFile(fileName, kXMP_UnknownFile, smartFlags);
    ok = xmpFile->OpenFile(fileName, kXMP_PDFFile, smartFlags);
    if (!ok) {
        LOGE("Failed to get a smart handler\n");
//        ok = xmpFile->OpenFile(fileName, kXMP_UnknownFile, scanFlags);
        ok = xmpFile->OpenFile(fileName, kXMP_PDFFile, scanFlags);
        LOGE("Try Open with scan, result: %s\n", ok ? "success" : "fail");
        if (!ok) return;
    }

    ok = xmpFile->GetFileInfo(0, &openFlags, &format, &handlerFlags);
    if (!ok) return;

    LOGE("File info : format = %.8X, handler flags = 0x%X, open flags = 0x%X (%s)\n",
         format, handlerFlags, openFlags, (isUpdate ? "update" : "read-only"));

    ok = xmpFile->GetXMP(xmpMeta, 0, &xmpPacket);
    if (!ok) {
        LOGE("No XMP\n");
        return;
    }

    LOGE("XMP packet info : file offset = %lld, length = %d, pad = %d",
         xmpPacket.offset, xmpPacket.length, xmpPacket.padSize);
    LOGE(", %s", ((xmpPacket.charForm > 5) ? "bad char form" : charForms[xmpPacket.charForm]));
    LOGE(", %s\n", (xmpPacket.writeable ? "writeable" : "read-only"));

    LOGE("\n");

    // Remove extaneous properties to make the dump smaller.
    SXMPUtils::RemoveProperties(xmpMeta, kXMP_NS_XMP, "Thumbnails", true);
    SXMPUtils::RemoveProperties(xmpMeta, kXMP_NS_XMP, "PageInfo", true);
    SXMPUtils::RemoveProperties(xmpMeta, "http://ns.adobe.com/xap/1.0/t/pg/", 0, true);
    SXMPUtils::RemoveProperties(xmpMeta, "http://ns.adobe.com/xap/1.0/mm/", 0, true);
#if 1
    SXMPUtils::RemoveProperties(xmpMeta, "http://ns.adobe.com/camera-raw-settings/1.0/", 0, true);
    SXMPUtils::RemoveProperties(xmpMeta, "http://ns.adobe.com/tiff/1.0/", 0, true);
    SXMPUtils::RemoveProperties(xmpMeta, "http://ns.adobe.com/exif/1.0/", 0, true);
    SXMPUtils::RemoveProperties(xmpMeta, "http://ns.adobe.com/exif/1.0/aux/", 0, true);
    SXMPUtils::RemoveProperties(xmpMeta, "http://ns.adobe.com/photoshop/1.0/", 0, true);
    SXMPUtils::RemoveProperties(xmpMeta, "http://ns.adobe.com/pdf/1.3/", 0, true);
#endif

}    // OpenTestFile

// -------------------------------------------------------------------------------------------------

#ifndef OnlyReadOnly
#define OnlyReadOnly 0
#endif

static void TestOneFile(const char *fileName) {
    char buffer[1000];
    SXMPMeta xmpMeta;
    SXMPFiles xmpFile;
    XMP_PacketInfo xmpPacket;
    std::string roDump, rwDump;

    sprintf(buffer, "Testing %s", fileName);
    WriteMinorLabel(sLogFile, buffer);

    OpenTestFile(fileName, kXMPFiles_OpenForRead, &xmpMeta, &xmpFile);
    xmpMeta.DumpObject(DumpToString, &roDump);
    xmpFile.CloseFile();

    if (OnlyReadOnly) {
        LOGE("Initial XMP from %s\n", fileName);
        xmpMeta.DumpObject(DumpToFile, sLogFile);
        return;
    }

    OpenTestFile(fileName, kXMPFiles_OpenForUpdate, &xmpMeta, &xmpFile);
    xmpMeta.DumpObject(DumpToString, &rwDump);
    if (roDump != rwDump) {
        LOGE("** Initial read-only and update XMP don't match! **\n\n");
        LOGE("Read-only XMP\n%s\nUpdate XMP\n%s\n", roDump.c_str(), rwDump.c_str());
    }

    LOGE("Initial XMP from %s\n", fileName);
    xmpMeta.DumpObject(DumpToFile, sLogFile);
    LOGE("DumpObject done.");

    XMP_DateTime now;
    SXMPUtils::CurrentDateTime(&now);
    std::string nowStr;
    SXMPUtils::ConvertFromDate(now, &nowStr);
    if (xmpMeta.CountArrayItems(kXMP_NS_XMP, "XMPFileStamps") >= 9) {
        xmpMeta.DeleteProperty(kXMP_NS_XMP, "XMPFileStamps");
    }
    xmpMeta.AppendArrayItem(kXMP_NS_XMP, "XMPFileStamps", kXMP_PropArrayIsOrdered, "");
    xmpMeta.SetProperty(kXMP_NS_XMP, "XMPFileStamps[last()]", nowStr.c_str());

    nowStr.insert(0, "Updating dc:description at ");
    xmpMeta.SetLocalizedText(kXMP_NS_DC, "description", "", "x-default", nowStr.c_str());

    xmpFile.PutXMP(xmpMeta);
    XMP_OptionBits closeOptions = 0;
    XMP_OptionBits capabilities = 0;
    XMP_FileFormat fileFormat = 0;
    xmpFile.GetFileInfo(NULL, NULL, &fileFormat, NULL);
    SXMPFiles::GetFormatInfo(fileFormat, &capabilities);

    if ((xmpMeta.CountArrayItems(kXMP_NS_XMP, "XMPFileStamps") & 1) == 0
        && (capabilities & kXMPFiles_AllowsSafeUpdate))
        closeOptions |= kXMPFiles_UpdateSafely;

    xmpFile.CloseFile(closeOptions);

    LOGE("\n");
    OpenTestFile(fileName, kXMPFiles_OpenForRead, &xmpMeta, &xmpFile);
    LOGE("Modified XMP from %s\n", fileName);
    xmpMeta.DumpObject(DumpToFile, sLogFile);
    xmpFile.CloseFile();

    LOGE("\nDone testing %s\n", fileName);

}    // TestOneFile

// -------------------------------------------------------------------------------------------------

extern "C" int tryUnitTestFile(const char *filename) {
    int result = 0;

//    char   logName[256];
    int nameLen = (int) strlen(filename);
    if ((nameLen >= 4) && (strcmp(filename + nameLen - 4, ".exe") == 0)) nameLen -= 4;
//    memcpy ( logName, filename, nameLen );
//    memcpy ( &logName[nameLen], "Log.txt", 8 );	// Include final null.
//    sLogFile = fopen ( logName, "wb" );

    time_t now = time(0);
    LOGW("XMPFilesCoverage open file %s starting %s", filename, ctime(&now));

    XMP_VersionInfo coreVersion, filesVersion;
    SXMPMeta::GetVersionInfo(&coreVersion);
    SXMPFiles::GetVersionInfo(&filesVersion);
    LOGE("Version :\n   %s\n   %s\n", coreVersion.message, filesVersion.message);

    try {

        if (!SXMPMeta::Initialize()) {
            LOGE("## XMPMeta::Initialize failed!\n");
            return -1;
        }
        XMP_OptionBits options = 0;
#if UNIX_ENV
        options |= kXMPFiles_ServerMode;
#endif
        if (!SXMPFiles::Initialize(options)) {
            LOGE("## SXMPFiles::Initialize failed!\n");
            return -1;
        }

        DumpHandlerInfo();

        TestOneFile(filename);

    } catch (XMP_Error &excep) {

        LOGE("\nCaught XMP_Error %d : %s\n", excep.GetID(), excep.GetErrMsg());
        result = -2;

    } catch (...) {

        LOGE("## Caught unknown exception\n");
        result = -3;

    }

    SXMPFiles::Terminate();
    SXMPMeta::Terminate();

    now = time(0);
    LOGW("\nXMPFilesCoverage finished %s", ctime(&now));
    LOGW("Final status = %d\n", result);
    fclose(sLogFile);

//    printf( "\nresults have been logged into %s\n", logName );

    return result;

}
