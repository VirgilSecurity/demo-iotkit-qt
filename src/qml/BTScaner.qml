/**
 * Copyright (C) 2015 Virgil Security Inc.
 *
 * Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     (1) Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *     (2) Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *
 *     (3) Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

import QtQuick 2.5
import com.virgil.cpp.app 1.0

Item {
    property alias mainList: mainList
    property var inProgress

    id: mainItem

    VSQNetifBLEEnumerator { id: bleEnum }

    Connections {
        target: bleEnum

        onFireDevicesListUpdated: {
            btScanerForm.mainList.model = bleEnum.devicesList()
        }

        onFireDiscoveryFinished: {
            btScanerForm.mainList.model = bleEnum.devicesList()
            btScanerForm.inProgress = "false"
        }
    }

    Rectangle {
        id: mainRect
        anchors.fill: parent
        color: "#155902"

        Rectangle {
            id: btnStop
            y: 262
            width: 100
            height: 35
            color: "#77b0e8"
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 8

            MouseArea {
                id: mouseAreaStop
                anchors.fill: parent

                onClicked:  {
                    inProgress = "false"
                }
            }

            Text {
                id: text1
                text: qsTr("Stop")
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                anchors.fill: parent
                font.pixelSize: 20
            }
        }

        Rectangle {
            id: btnConnect
            x: 152
            y: 262
            width: 100
            height: 35
            color: "#c9a1e2"
            visible: inProgress != "true"
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 8
            anchors.right: parent.right
            anchors.rightMargin: 8

            Text {
                id: text2
                text: qsTr("Connect")
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                anchors.fill: parent
                font.pixelSize: 20

                MouseArea {
                    id: mouseAreaConnect
                    anchors.fill: parent
                }
            }
        }

        Rectangle {
            id: btnRefresh
            x: 150
            y: 357
            width: 100
            height: 35
            color: "#f98f66"
            visible: inProgress != "true"
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 8
            anchors.horizontalCenter: parent.horizontalCenter

            Text {
                id: text3
                text: qsTr("Refresh")
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                anchors.fill: parent
                font.pixelSize: 20

                MouseArea {
                    id: mouseAreaRefresh
                    anchors.fill: parent
                    onClicked: {
                        startDiscovery()
                        inProgress = "true"
                    }
                }
            }
        }
    }

    Rectangle {
        id: busy
        height: 22

        anchors.top: parent.top
        radius: 5
        anchors.topMargin: 8
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.left: parent.left
        anchors.leftMargin: 8
        color: "#1c56f3"
        visible: inProgress == "true"

        SequentialAnimation on color {
            id: busyThrobber
            ColorAnimation { easing.type: Easing.InOutSine; from: "#1c56f3"; to: "white"; duration: 1000; }
            ColorAnimation { easing.type: Easing.InOutSine; to: "#1c56f3"; from: "white"; duration: 1000 }
            loops: Animation.Infinite
        }
    }

    ListView {
        id: mainList
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 53
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.left: parent.left
        anchors.leftMargin: 8
        anchors.top: busy.bottom
        anchors.topMargin: 6

        delegate: Rectangle {
            property variant selectedData: model

            id: btDelegate
            width: parent.width
            height: column.height + 10

            clip: true
            Image {
                id: bticon
                source: "qrc:/default.png";
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

    function selectedDevice() {
        return mainList.currentItem.selectedData.modelData
    }

    function startDiscovery() {
        bleEnum.startDiscovery()
        btScanerForm.mainList.model = bleEnum.devicesList()
        btScanerForm.inProgress = "true"
    }
}

