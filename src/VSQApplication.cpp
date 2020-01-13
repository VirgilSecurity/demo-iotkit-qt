//  Copyright (C) 2015-2020 Virgil Security, Inc.
//
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//      (1) Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//      (2) Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in
//      the documentation and/or other materials provided with the
//      distribution.
//
//      (3) Neither the name of the copyright holder nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
//  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
//  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
//  Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>

#include <QtCore>
#include <QtQml>

#include <VSQApplication.h>

#include <virgil/iot/qt/VSQIoTKit.h>
#include <virgil/iot/qt/netif/VSQUdpBroadcast.h>
#include <virgil/iot/qt/netif/VSQNetifBLE.h>
#include <virgil/iot/qt/netif/VSQNetifBLEEnumerator.h>
#include <virgil/iot/logger/logger.h>

int
VSQApplication::run() {
    QQmlApplicationEngine engine;
    VSQNetifBLEEnumerator bleEnumerator;
    auto netifBLE = QSharedPointer<VSQNetifBLE>::create();

    auto features = VSQFeatures() << VSQFeatures::SNAP_INFO_CLIENT << VSQFeatures::SNAP_SNIFFER;
    auto impl = VSQImplementations() << netifBLE;
    auto roles = VSQDeviceRoles() << VirgilIoTKit::VS_SNAP_DEV_CONTROL;
    auto appConfig = VSQAppConfig() << VSQManufactureId() << VSQDeviceType() << VSQDeviceSerial()
                                    << VirgilIoTKit::VS_LOGLEV_DEBUG << roles << VSQSnapSnifferQmlConfig();

    // Connect signals and slots
    QObject::connect(&bleEnumerator, &VSQNetifBLEEnumerator::fireDeviceSelected,
            netifBLE.data(), &VSQNetifBLE::onOpenDevice);

    QObject::connect(netifBLE.data(), SIGNAL(fireDeviceReady()),
                     &VSQSnapInfoClient::instance(), SLOT(onStartFullPolling()));

    // Initialize IoTKit
    if (!VSQIoTKitFacade::instance().init(features, impl, appConfig)) {
        VS_LOG_CRITICAL("Unable to initialize Virgil IoT KIT");
        return -1;
    }

    QQmlContext *context = engine.rootContext();
    context->setContextProperty("bleEnum", &bleEnumerator);
    context->setContextProperty("SnapInfoClient", &VSQSnapInfoClientQml::instance());
    context->setContextProperty("SnapSniffer", VSQIoTKitFacade::instance().snapSniffer().get());

    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    engine.load(url);

    // Change size of window for desctop version
#if defined (__APPLE__) || defined (__linux)
    QObject * rootObject(engine.rootObjects().first());
    rootObject->setProperty("width", 600);
    rootObject->setProperty("height", 400);
#endif

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS) && !defined(Q_OS_WATCHOS)
    {
        QObject *rootObject(engine.rootObjects().first());

        int prevWidth = rootObject->property("width").toInt();
        int prevHeight = rootObject->property("height").toInt();
        int prevX = rootObject->property("x").toInt();
        int prevY = rootObject->property("y").toInt();

        constexpr int newWidth = 640;
        constexpr int newHeight = 400;
        int newX = prevX - ( newWidth - prevWidth ) / 2;
        int newY = prevY - ( newHeight - prevHeight ) / 2;

        rootObject->setProperty("width", newWidth);
        rootObject->setProperty("height", newHeight);
        rootObject->setProperty("x", newX);
        rootObject->setProperty("y", newY);
    }
#endif

    return QGuiApplication::instance()->exec();
}
