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

import QtQuick 2.5
import QtQuick.Layouts 1.5
import QtQuick.Controls 2.12

Item {
    property alias mainList: mainList

    id: mainItem

    Connections {
        target: bleEnum

        onFireDevicesListUpdated: {
            btScanerForm.mainList.model = bleEnum.devicesList()
        }

        onFireDiscoveryFinished: {
            btScanerForm.mainList.model = bleEnum.devicesList()
            startDiscovery();
        }
    }

    Rectangle {
        id: mainRect
        anchors.fill: parent
        color: "#155902"

        Rectangle {
            id: btnInitialize
            width: 100
            height: 35
            color: "#c9a1e2"
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 8
            anchors.horizontalCenter: parent.horizontalCenter

            Text {
                id: txtInitialize
                text: qsTr("Initialize")
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                anchors.fill: parent
                font.pixelSize: 20

                MouseArea {
                    id: mouseAreaCInitialize
                    anchors.fill: parent

                    onClicked: {
                        onClicked: showMessageBox()
                    }
                }
            }
        }
    }

    ListView {
        id: mainList
        anchors.fill: parent
        anchors.bottomMargin: 53
        anchors.rightMargin: 8
        anchors.leftMargin: 8
        anchors.topMargin: 8

        delegate: Rectangle {
            property variant selectedData: model

            id: btDelegate
            width: parent.width
            height: column.height + 10

            clip: true
            Image {
                id: bticon
                source: "qrc:/qml/default.png";
                width: bttext.height;
                height: bttext.height;
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 5
            }

            Column {
                id: column
                anchors.left: bticon.right
                anchors.leftMargin: 5
                Text {
                    id: bttext
                    text: modelData
                    font.family: "FreeSerif"
                    font.pointSize: 16
                }
            }

            color: ListView.view.currentIndex === index ? "steelblue" : "white"

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    mainList.currentIndex = index
                }
            }
        }

        focus: true
    }

    Component.onCompleted: {
        startDiscovery();
    }

    function selectedDevice() {
        return mainList.currentItem.selectedData.modelData
    }

    function startDiscovery() {
        bleEnum.startDiscovery()
        btScanerForm.mainList.model = bleEnum.devicesList()
    }

    function showMessageBox() {
        var component = Qt.createComponent("InitDialog.qml")
        if (component.status === Component.Ready) {
            var dialog = component.createObject(applicationWindow)
            dialog.applied.connect(function()
                    {
                        initializeDevice()
                        dialog.close()
                    })
            dialog.open()
            return dialog
        }
        console.error(component.errorString())
        return null
    }

    function initializeDevice() {
        try {
            var deviceName = btScanerForm.selectedDevice();
            bleEnum.select(deviceName);
        } catch (error) {
            console.error("Cannot start initialization of device")
        }
    }
}

