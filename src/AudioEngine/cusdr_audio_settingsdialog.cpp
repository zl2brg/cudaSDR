/**
* @file cusdr_audio_settingsdialog.cpp
* @brief cuSDR audio settings dialogue class
* @author adaptation for cuSDR by Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2011-04-02
* Updated 2025-06-10 for qt6
*/

/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include "cusdr_audio_settingsdialog.h"
#include "Util/cusdr_buttons.h"
#include "Util/AudioDeviceService.h"


#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>

#define	btn_height		18
#define	btn_width		74

SettingsDialog::SettingsDialog(QWidget *parent)
    :   QDialog(parent)
	,	set(Settings::instance())
 {
    AudioDeviceService* audioService = AudioDeviceService::instance();

	if (parent)
		setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
	else
		setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

	setWindowModality(Qt::NonModal);
	setWindowOpacity(0.9);

	setMouseTracking(true);

	m_titleFont.setStyleStrategy(QFont::PreferAntialias);
	m_titleFont.setFixedPitch(true);
	#ifdef Q_OS_MAC
		m_titleFont.setPixelSize(10);
		m_titleFont.setFamily("Arial");
	#endif
	#ifdef Q_OS_WIN
		m_titleFont.setPixelSize(13);
		m_titleFont.setFamily("Arial");
		m_titleFont.setBold(true);
	#endif

    QVBoxLayout *dialogLayout = new QVBoxLayout(this);

    m_inputDeviceComboBox.setMinimumContentsLength(30);
    m_outputDeviceComboBox.setMinimumContentsLength(30);

    // Initialize default devices
    m_inputDevice = audioService->defaultInput();
    m_outputDevice = audioService->defaultOutput();

    // Add widgets to layout

	QHBoxLayout* titleLayout = new QHBoxLayout;
	QLabel *titleLabel = new QLabel(tr("Audio Settings:"), this);
	titleLabel->setFont(m_titleFont);
    titleLayout->addWidget(titleLabel);
    dialogLayout->addLayout(titleLayout);

    QHBoxLayout* inputDeviceLayout = new QHBoxLayout;
    QLabel *inputDeviceLabel = new QLabel(tr("Input device"), this);
    inputDeviceLayout->addWidget(inputDeviceLabel);
    inputDeviceLayout->addWidget(&m_inputDeviceComboBox);
    dialogLayout->addLayout(inputDeviceLayout);

    QHBoxLayout* outputDeviceLayout = new QHBoxLayout;
    QLabel *outputDeviceLabel = new QLabel(tr("Output device"), this);
    outputDeviceLayout->addWidget(outputDeviceLabel);
    outputDeviceLayout->addWidget(&m_outputDeviceComboBox);
    dialogLayout->addLayout(outputDeviceLayout);

    // Connect
    CHECKED_CONNECT(
        &m_inputDeviceComboBox,
		qOverload<int>(&QComboBox::activated),
        this, 
		&SettingsDialog::inputDeviceChanged);

    CHECKED_CONNECT(
        &m_outputDeviceComboBox,
		qOverload<int>(&QComboBox::activated),
        this, 
		&SettingsDialog::outputDeviceChanged);

    connect(audioService, &AudioDeviceService::audioInputsChanged, this, &SettingsDialog::getAudioDevices);
    connect(audioService, &AudioDeviceService::audioOutputsChanged, this, &SettingsDialog::getAudioDevices);

	AeroButton* okBtn = new AeroButton("Ok", this);
	okBtn->setRoundness(10);
	okBtn->setFixedSize(btn_width, btn_height);
	CHECKED_CONNECT(
		okBtn, 
		&AeroButton::clicked, 
		this, 
		&SettingsDialog::accept);

	AeroButton* cancelBtn = new AeroButton("Cancel", this);
	cancelBtn->setRoundness(10);
	cancelBtn->setFixedSize(btn_width, btn_height);
	CHECKED_CONNECT(
		cancelBtn, 
		&AeroButton::clicked, 
		this, 
		&SettingsDialog::reject);

	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->setSpacing(1);
	hbox->addWidget(okBtn);
	hbox->addWidget(cancelBtn);

	dialogLayout->addLayout(hbox);
    
    setLayout(dialogLayout);
    getAudioDevices();
}

SettingsDialog::~SettingsDialog() {
}

void SettingsDialog::inputDeviceChanged(int index) {
    m_inputDevice = m_inputDeviceComboBox.itemData(index).value<QAudioDevice>();
    
    // index 0 is "hpsdr-local", so we need to offset the Qt device index by 1.
    set->setMicInputDev(index + 1);
    set->setMicInputSourceName(m_inputDevice.description());
}

void SettingsDialog::outputDeviceChanged(int index) {
    m_outputDevice = m_outputDeviceComboBox.itemData(index).value<QAudioDevice>();
    // set->setOutputDev(index); // Assuming there's a setting for this
}

void SettingsDialog::getAudioDevices() {
    AudioDeviceService* audioService = AudioDeviceService::instance();
    
    m_inputDeviceComboBox.clear();
    m_outputDeviceComboBox.clear();

    QStringList inputs = audioService->audioInputDescriptions();
    for (int i = 0; i < inputs.size(); ++i) {
        m_inputDeviceComboBox.addItem(inputs[i], QVariant::fromValue(audioService->audioInputs()[i]));
    }

    QStringList outputs = audioService->audioOutputDescriptions();
    for (int i = 0; i < outputs.size(); ++i) {
        m_outputDeviceComboBox.addItem(outputs[i], QVariant::fromValue(audioService->audioOutputs()[i]));
    }
}

