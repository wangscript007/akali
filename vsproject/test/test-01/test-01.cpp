#include "stdafx.h"

#include <thread>
#include <chrono>
#include <atomic>
#include <iostream>
#include "StringTest.h"
#include "IPCTest.h"

std::atomic<bool> over;
std::atomic<ppx::net::FileTransferBase::Status> g_state;

void StatusCB(base::String filePath, ppx::net::FileTransferBase::Status state, base::String reason, int64_t used_millsec) {
    g_state = state;

    if (g_state == ppx::net::FileTransferBase::Status::Finished || g_state == ppx::net::FileTransferBase::Status::Failed) {
        over = true;
    }
}

void ProgressCB(base::String filePath, int64_t total, int64_t transfered) {
    static double last_per = 0.f;
    if (total > 0) {
        double per = ((double)transfered / (double)total * 100.0);
        if (per - last_per > 0.1f || total == transfered) {
            last_per = per;
            printf("%%%.1f\n", per);
        }
    }
    else {
        printf("--\n");
    }
}


int main() {
    ppx::base::CmdLineParser cml(GetCommandLine());
    if (cml.HasKey("ipc_s")) {
        IPCTester test_s;
        test_s.StartIPCServer(TCHARToAnsi(cml.GetVal("ipc_s")));
        getchar();
        test_s.StopIPCServer();
        getchar();
        return 0;
    }
    else if (cml.HasKey("ipc_c")) {
        IPCTester test_c;
        test_c.StartIPCServer(TCHARToAnsi(cml.GetVal("ipc_c")));
        test_c.BatchSend(TCHARToAnsi(cml.GetVal("target")));

        getchar();
        test_c.StopIPCServer();
        getchar();
        return 0;
    }

    bool bDelRet = ppx::base::RegKey::DeleteKey(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"WallpaperEngine", false);
	StringTest();

    int thread_nums[4] = { 1,2,3,10 };
    bool interruption_resuming[2] = { false, true };

#define url_num 1
    std::string urls[url_num] = {
        "http://pre-api.steamboxs.com/avatar/13.png",
    };

    std::string md5s[url_num] = {
        "b66883aaed85d10120434bb91b119721",
    };
    for (int i = 0; i < sizeof(thread_nums) / sizeof(int); i++) {
        for (int j = 0; j < sizeof(interruption_resuming) / sizeof(bool); j++) {
            for (int k = 0; k < url_num; k++) {
                over = false;
                PPX_LOG(LS_INFO) << "**** [" << urls[k] << "] thread_num = " << thread_nums[i] << ", interruption_resuming = " << interruption_resuming[j];
                ppx::net::FileDownload fileDownload;
                fileDownload.SetUrl(urls[k]);
                fileDownload.SetFileDir("D:\\");
                fileDownload.SetFileName("test");
                fileDownload.SetFileExt(".png");
                fileDownload.SetThreadNum(thread_nums[i]);
                fileDownload.SetFileMd5(md5s[k]);
                fileDownload.SetProgressInterval(100);
                fileDownload.SetInterruptionResuming(interruption_resuming[j]);
                fileDownload.SetStatusCallback(std::bind(StatusCB, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
                fileDownload.SetProgressCallback(std::bind(ProgressCB, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
                fileDownload.Start();

                while (!over) {
                    Sleep(100);
                }

                if (g_state == ppx::net::FileTransferBase::Status::Finished) {
                    PPX_LOG(LS_INFO) << "**** [" << urls[k] << "] OK";
                }
                else {
                    PPX_LOG(LS_INFO) << "**** [" << urls[k] << "] FAILED";
                    getchar();
                }
            }
        }
    }

    PPX_LOG(LS_INFO) << "**** TEST END";


	base::BufferQueue resp;
	net::HttpRequest request;
	int iHttpCode = request.Get("https://www.baidu.com", resp);

	char* pBuf = NULL;
	int64_t iBufSize = resp.ToOneBuffer(&pBuf);

	if (iBufSize > 0 && pBuf) {
		FILE *f;
		fopen_s(&f, "D:\\test.html", "wb");
		fwrite(pBuf, 1, resp.GetTotalDataSize(), f);
		fclose(f);

		printf("%s\n", pBuf);
		base::TraceMsgA("\n%s\n", pBuf);
	}

	if (pBuf)
		free(pBuf);

    getchar();
    return 0;
}
