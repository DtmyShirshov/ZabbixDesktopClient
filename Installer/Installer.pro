# Before run make project, need to put executable file and dlls into
# $$PWD/installer/packages/ru.motomoto.ZBCinstaller/data

TEMPLATE = aux

# В зависимости от режима сборки, определяем, куда именно будут собираться инсталляторы
CONFIG(release, debug|release) {
    INSTALLER_OFFLINE = $$OUT_PWD/../../InstallerRelease/installer
    DESTDIR_WIN = $$PWD/packages/ru.motomoto.ZBCinstaller/data
    DESTDIR_WIN ~= s,/,\\,g
    PWD_WIN = $$OUT_PWD/../../ClientRelease
    PWD_WIN ~= s,/,\\,g

    copydata.commands = $(COPY_DIR) $$PWD_WIN $$DESTDIR_WIN
    first.depends = $(first) copydata
    export(first.depends)
    export(copydata.commands)
    QMAKE_EXTRA_TARGETS += first copydata
}

# Создаём цель по сборке Оффлайн Инсталлятора
INPUT = $$PWD/config/config.xml $$PWD/packages
offlineInstaller.depends = copydata
offlineInstaller.input = INPUT
offlineInstaller.output = $$INSTALLER_OFFLINE
offlineInstaller.commands = $$(QTDIR)/../../QtIFW2.0.1/bin/binarycreator --offline-only -c $$PWD/config/config.xml -p $$PWD/packages ${QMAKE_FILE_OUT}
offlineInstaller.CONFIG += target_predeps no_link combine

QMAKE_EXTRA_COMPILERS += offlineInstaller

DISTFILES += \
    packages/ru.motomoto.ZBCinstaller/meta/installscript.qs \
    packages/ru.motomoto.ZBCinstaller/meta/package.xml



