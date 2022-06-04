package com.aiwannafly.gui_torrent.controller;

import com.aiwannafly.gui_torrent.view.GUITorrentRenderer;
import javafx.application.Platform;

import java.util.concurrent.Flow;

public class UploadListener implements Flow.Subscriber<Boolean> {
    private final GUITorrentRenderer.FileSection fileSection;
    private long lastUpdateTime = System.currentTimeMillis();
    private long currentTime = System.currentTimeMillis();
    private final int piecesPortion;
    private int uploadedCount = 0;

    public UploadListener(GUITorrentRenderer.FileSection fileSection) {
        this.fileSection = fileSection;
        this.piecesPortion = 1 + fileSection.torrent.getPieces().size() / 10;
    }

    @Override
    public void onSubscribe(Flow.Subscription subscription) {

    }

    @Override
    public void onNext(Boolean item) {
        Platform.runLater(this::showUploadSpeed);
    }

    @Override
    public void onError(Throwable throwable) {

    }

    @Override
    public void onComplete() {

    }

    private void showUploadSpeed() {
        uploadedCount++;
        if (uploadedCount % piecesPortion != 0) {
            return;
        }
        currentTime = System.currentTimeMillis();
        long speedKB = calcUploadSpeed();
        lastUpdateTime = currentTime;
        fileSection.uSpeedLabel.setText(String.valueOf(speedKB));
    }

    private long calcUploadSpeed() {
        double estimatedTime = currentTime - lastUpdateTime;
        long uploadedBytes = piecesPortion * fileSection.torrent.getPieceLength();
        double estimatedTimeSecs = estimatedTime / 1000;
        double speed = uploadedBytes / estimatedTimeSecs;
        return (long) speed / 1024;
    }
}
