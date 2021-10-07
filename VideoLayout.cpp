#pragma execution_character_set("utf-8")
#include "VideoLayout.h"

void setParentDesktop(QWidget* pWidget) {
    // �ο���https://www.codeproject.com/articles/856020/draw-behind-desktop-icons-in-windows
    SendMessageTimeout(FindWindow(L"ProgMan", NULL), 0x52C, NULL, NULL, SMTO_NORMAL, 1000, NULL); // �ٻ� WorkerW ��
    HWND hDefView = NULL;
    HWND hWorkerW = FindWindow(L"WorkerW", NULL);
    while (!hDefView && hWorkerW) { // WorkerW ���ж����Ѱ�Һ��� SHELLDLL_DefView �����һ�� Worker ��
        hDefView = FindWindowEx(hWorkerW, NULL, L"SHELLDLL_DefView", NULL);
        hWorkerW = FindWindowEx(NULL, hWorkerW, L"WorkerW", NULL);
    }
    if (hWorkerW) {
        ShowWindow(hWorkerW, SW_HIDE);
    }
    HWND hDesktop = FindWindow(L"Progman", NULL);
    if (hDesktop) {
        SetParent((HWND)pWidget->winId(), hDesktop);
    }
}

VideoLayout::VideoLayout(QWidget* parent) : QWidget(parent) {
    ui.setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(QGuiApplication::primaryScreen()->availableGeometry().size()); // �����Ƕ���ʾ������Ϊ��û��
    setParentDesktop(this);

    ui.video->setFixedSize(size());
}

VideoLayout::~VideoLayout() {
    libvlc_media_player_stop(mediaPlayer);
    libvlc_media_list_player_stop(mediaListPlayer);

    libvlc_release(instance);
    libvlc_media_list_release(mediaList);
    libvlc_media_player_release(mediaPlayer);
    libvlc_media_list_player_release(mediaListPlayer);
}

bool VideoLayout::play(const QString& videoPath, bool change) {
    if (change) {
        libvlc_release(instance);
        libvlc_media_list_release(mediaList);
        libvlc_media_player_release(mediaPlayer);
        libvlc_media_list_player_release(mediaListPlayer);
    }

    instance = libvlc_new(0, NULL);
    if (instance == 0) {
        QMessageBox::critical(this, "����", "��ʼ��instanceʧ��", QMessageBox::Ok);
        return false;
    }
    mediaList = libvlc_media_list_new(instance);
    if (mediaList == 0) {
        QMessageBox::critical(this, "����", "��ʼ��mediaListʧ��", QMessageBox::Ok);
        return false;
    }

    libvlc_media_t* media = libvlc_media_new_location(instance, ("file:///" + videoPath).toLatin1()); // ʹ�� libvlc_media_new_path ���Ҳ����ļ�
    if (media == 0) {
        QMessageBox::critical(this, "����", "��ʼ��mediaʧ��", QMessageBox::Ok);
        return false;
    }
    if (libvlc_media_list_add_media(mediaList, media) != 0) {
        QMessageBox::critical(this, "����", "���ý��ʧ��", QMessageBox::Ok);
        return false;
    }
    libvlc_media_release(media);

    mediaPlayer = libvlc_media_player_new(instance);
    if (mediaPlayer == 0) {
        QMessageBox::critical(this, "����", "��ʼ��mediaPlayerʧ��", QMessageBox::Ok);
        return false;
    }
    mediaListPlayer = libvlc_media_list_player_new(instance);
    if (mediaListPlayer == 0) {
        QMessageBox::critical(this, "����", "��ʼ��mediaListPlayerʧ��", QMessageBox::Ok);
        return false;
    }

    libvlc_media_list_player_set_media_list(mediaListPlayer, mediaList);
    libvlc_media_list_player_set_media_player(mediaListPlayer, mediaPlayer);
    libvlc_media_list_player_set_playback_mode(mediaListPlayer, libvlc_playback_mode_loop);

    libvlc_media_player_set_hwnd(mediaPlayer, (HWND)ui.video->winId());
    libvlc_media_list_player_play(mediaListPlayer);

    return true;
}

void VideoLayout::pause() {
    if (libvlc_media_list_player_get_state(mediaListPlayer) == libvlc_Ended) {
        libvlc_media_list_player_stop(mediaListPlayer);
        libvlc_media_list_player_play(mediaListPlayer);
    }
    else {
        libvlc_media_list_player_pause(mediaListPlayer);
    }
}

void VideoLayout::loopMode(bool enable) {
    if (enable) {
        libvlc_media_list_player_set_playback_mode(mediaListPlayer, libvlc_playback_mode_loop);
    }
    else {
        libvlc_media_list_player_set_playback_mode(mediaListPlayer, libvlc_playback_mode_default);
    }
}