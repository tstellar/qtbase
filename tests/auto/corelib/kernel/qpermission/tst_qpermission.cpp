// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QPermission>

#include <QTest>

struct DummyPermission // a minimal QPermission-compatible type
{
    using QtPermissionHelper = void;
    int state = 0;
};
Q_DECLARE_METATYPE(DummyPermission)

class tst_QPermission : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void converting_Dummy() const { return converting_impl<DummyPermission>(); }
    void converting_Location() const { return converting_impl<QLocationPermission>(); }
    void converting_Calendar() const { return converting_impl<QCalendarPermission>(); }
    void converting_Contacts() const { return converting_impl<QContactsPermission>(); }
    void converting_Camera() const { return converting_impl<QCameraPermission>(); }
    void converting_Microphone() const { return converting_impl<QMicrophonePermission>(); }
    void converting_Bluetooth() const { return converting_impl<QBluetoothPermission>(); }

    void conversionMaintainsState() const;
private:
    template <typename T>
    void converting_impl() const;
};

template <typename T>
void tst_QPermission::converting_impl() const
{
    T concrete;
    const T cconcrete = concrete;
    const auto metaType = QMetaType::fromType<T>();

    // construction is implicit:
    // from rvalue:
    {
        QPermission p = T();
        QCOMPARE_EQ(p.type(), metaType);
    }
    // from mutable lvalue:
    {
        QPermission p = concrete;
        QCOMPARE_EQ(p.type(), metaType);
    }
    // from const lvalue:
    {
        QPermission p = cconcrete;
        QCOMPARE_EQ(p.type(), metaType);
    }

    // value<>() compiles:
    {
        const QPermission p = concrete;
        auto v = p.value<T>();
        static_assert(std::is_same_v<decltype(v), std::optional<T>>);
        QCOMPARE_NE(v, std::nullopt);
    }
}

void tst_QPermission::conversionMaintainsState() const
{
    DummyPermission dummy{42}, dummy_default;
    QCOMPARE_NE(dummy.state, dummy_default.state);

    QLocationPermission loc, loc_default;
    QCOMPARE_EQ(loc_default.accuracy(), QLocationPermission::Accuracy::Approximate);
    QCOMPARE_EQ(loc_default.availability(), QLocationPermission::Availability::WhenInUse);

    loc.setAccuracy(QLocationPermission::Accuracy::Precise);
    loc.setAvailability(QLocationPermission::Availability::Always);

    QCOMPARE_EQ(loc.accuracy(), QLocationPermission::Accuracy::Precise);
    QCOMPARE_EQ(loc.availability(), QLocationPermission::Availability::Always);

    QCalendarPermission cal, cal_default;
    QCOMPARE_EQ(cal_default.isReadWrite(), false);

    cal.setReadWrite(true);

    QCOMPARE_EQ(cal.isReadWrite(), true);

    QContactsPermission con, con_default;
    QCOMPARE_EQ(con_default.isReadWrite(), false);

    con.setReadWrite(true);

    QCOMPARE_EQ(con.isReadWrite(), true);

    //
    // QCameraPermission, QMicrophonePermission, QBluetoothPermission don't have
    // state at the time of writing
    //

    QPermission p; // maintain state between the blocks below to test reset behavior

    {
        p = dummy;
        auto v = p.value<DummyPermission>();
        QCOMPARE_NE(v, std::nullopt);
        auto &r = *v;
        QCOMPARE_EQ(r.state, dummy.state);
        // check mismatched returns nullopt:
        QCOMPARE_EQ(p.value<QCalendarPermission>(), std::nullopt);
    }

    {
        p = loc;
        auto v = p.value<QLocationPermission>();
        QCOMPARE_NE(v, std::nullopt);
        auto &r = *v;
        QCOMPARE_EQ(r.accuracy(), loc.accuracy());
        QCOMPARE_EQ(r.availability(), loc.availability());
        // check mismatched returns nullopt:
        QCOMPARE_EQ(p.value<DummyPermission>(), std::nullopt);
    }

    {
        p = con;
        auto v = p.value<QContactsPermission>();
        QCOMPARE_NE(v, std::nullopt);
        auto &r = *v;
        QCOMPARE_EQ(r.isReadWrite(), con.isReadWrite());
        // check mismatched returns nullopt:
        QCOMPARE_EQ(p.value<QLocationPermission>(), std::nullopt);
    }

    {
        p = cal;
        auto v = p.value<QCalendarPermission>();
        QCOMPARE_NE(v, std::nullopt);
        auto &r = *v;
        QCOMPARE_EQ(r.isReadWrite(), cal.isReadWrite());
        // check mismatched returns nullopt:
        QCOMPARE_EQ(p.value<QContactsPermission>(), std::nullopt);
    }
}

QTEST_APPLESS_MAIN(tst_QPermission)
#include "tst_qpermission.moc"
