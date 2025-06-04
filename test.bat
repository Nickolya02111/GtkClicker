@echo off
setlocal

:: --- НАСТРОЙКИ ---
:: Укажите полный путь к вашей установке MSYS2.
:: Пример: C:\msys64
set "MSYS2_ROOT=C:\msys64"

:: Укажите корень вашей MinGW64/UCRT64 среды внутри MSYS2.
:: Если вы используете MinGW64: set "MINGW_ENV_ROOT=%MSYS2_ROOT%\mingw64"
:: Если вы используете UCRT64 (рекомендуется): set "MINGW_ENV_ROOT=%MSYS2_ROOT%\ucrt64"
set "MINGW_ENV_ROOT=%MSYS2_ROOT%\mingw64"

:: Имя вашей программы (без пути, но с .exe)
set "APP_EXE_NAME=clicker.exe"

:: Путь к вашему исходному файлу .exe (относительно текущей директории MSYS2, где вы будете запускать скрипт,
:: или полный путь, если .exe находится не в текущей рабочей директории).
:: Предполагаем, что .exe находится в той же папке, что и этот .bat файл.
set "SOURCE_APP_PATH=%CD%\%APP_EXE_NAME%"

:: Имя папки, куда будут скопированы все файлы.
set "RELEASE_DIR_NAME=Clicker_Release"

:: Имена ваших дополнительных файлов (аудио, сохранение, иконка).
:: Укажите полный путь к ним, если они не находятся в текущей директории запуска скрипта.
set "AUDIO_FILE_NAME=t.mp3"
set "SAVE_FILE_NAME=save"
set "ICON_FILE_NAME=j.jpg"

:: --- НЕ МЕНЯЙТЕ НИЖЕ, ЕСЛИ НЕ ПОНИМАЕТЕ, ЧТО ДЕЛАЕТЕ ---

set "RELEASE_PATH=%CD%\%RELEASE_DIR_NAME%"
set "MINGW_BIN_PATH=%MINGW_ENV_ROOT%\bin"
set "GSTREAMER_LIB_PATH=%MINGW_ENV_ROOT%\lib\gstreamer-1.0"
set "GSTREAMER_RELEASE_PLUGINS_PATH=%RELEASE_PATH%\lib\gstreamer-1.0"

echo.
echo === Автоматическое копирование зависимостей для %APP_EXE_NAME% ===
echo.
echo Исходная папка MSYS2: %MSYS2_ROOT%
echo Рабочая среда MinGW: %MINGW_ENV_ROOT%
echo Исполняемый файл: %SOURCE_APP_PATH%
echo Целевая папка для релиза: %RELEASE_PATH%
echo.

:: 1. Удаляем предыдущую папку релиза (если она существует)
echo Удаление старой папки релиза: %RELEASE_PATH%
if exist "%RELEASE_PATH%" rd /s /q "%RELEASE_PATH%"
md "%RELEASE_PATH%"
if not exist "%RELEASE_PATH%" (
    echo Ошибка: Не удалось создать папку релиза. Выход.
    goto :eof
)

:: 2. Копируем исполняемый файл приложения
echo Копирование %APP_EXE_NAME%...
copy /y "%SOURCE_APP_PATH%" "%RELEASE_PATH%" >nul
if not exist "%RELEASE_PATH%\%APP_EXE_NAME%" (
    echo Ошибка: Не удалось скопировать %APP_EXE_NAME%. Убедитесь, что он существует по пути: %SOURCE_APP_PATH%
    goto :eof
)

:: 3. Копируем дополнительные файлы
echo Копирование дополнительных файлов...
if exist "%AUDIO_FILE_NAME%" copy /y "%AUDIO_FILE_NAME%" "%RELEASE_PATH%" >nul
if exist "%SAVE_FILE_NAME%" copy /y "%SAVE_FILE_NAME%" "%RELEASE_PATH%" >nul
if exist "%ICON_FILE_NAME%" copy /y "%ICON_FILE_NAME%" "%RELEASE_PATH%" >nul

:: 4. Список DLL-зависимостей, которые нужно скопировать
::    (Исключены системные DLL из C:\Windows, они есть у всех)
::    Этот список взят из вашего вывода ldd.
set "DLL_LIST=" ^
    "libgcc_s_seh-1.dll" ^
    "libatkmm-1.6-1.dll" ^
    "libglibmm-2.4-1.dll" ^
    "libgstreamer-1.0-0.dll" ^
    "libgtkmm-3.0-1.dll" ^
    "libsigc-2.0-0.dll" ^
    "libstdc++-6.dll" ^
    "libwinpthread-1.dll" ^
    "libatk-1.0-0.dll" ^
    "libgobject-2.0-0.dll" ^
    "libglib-2.0-0.dll" ^
    "libintl-8.dll" ^
    "libgmodule-2.0-0.dll" ^
    "libcairomm-1.0-1.dll" ^
    "libgdk-3-0.dll" ^
    "libgiomm-2.4-1.dll" ^
    "libgtk-3-0.dll" ^
    "libpangomm-1.4-1.dll" ^
    "libgdkmm-3.0-1.dll" ^
    "libffi-8.dll" ^
    "libiconv-2.dll" ^
    "libcairo-2.dll" ^
    "libgio-2.0-0.dll" ^
    "libcairo-gobject-2.dll" ^
    "libepoxy-0.dll" ^
    "libfribidi-0.dll" ^
    "libgdk_pixbuf-2.0-0.dll" ^
    "libpangocairo-1.0-0.dll" ^
    "libpangowin32-1.0-0.dll" ^
    "libpango-1.0-0.dll" ^
    "libpcre2-8-0.dll" ^
    "libharfbuzz-0.dll" ^
    "libfontconfig-1.dll" ^
    "libfreetype-6.dll" ^
    "libpixman-1-0.dll" ^
    "libpng16-16.dll" ^
    "zlib1.dll" ^
    "libjpeg-8.dll" ^
    "libtiff-6.dll" ^
    "libpangoft2-1.0-0.dll" ^
    "libthai-0.dll" ^
    "libgraphite2.dll" ^
    "libexpat-1.dll" ^
    "libbrotlidec.dll" ^
    "libbz2-1.dll" ^
    "libjbig-0.dll" ^
    "libdeflate.dll" ^
    "liblzma-5.dll" ^
    "libLerc.dll" ^
    "libwebp-7.dll" ^
    "libzstd.dll" ^
    "libdatrie-1.dll" ^
    "libbrotlicommon.dll" ^
    "libsharpyuv-0.dll"

echo Копирование необходимых DLLs...
for %%d in (%DLL_LIST%) do (
    set "DLL_SOURCE_PATH=%MINGW_BIN_PATH%\%%d"
    if exist "!DLL_SOURCE_PATH!" (
        copy /y "!DLL_SOURCE_PATH!" "%RELEASE_PATH%" >nul
        echo - Скопировано: %%d
    ) else (
        echo - Предупреждение: DLL не найдена: %%d (проверьте путь и установку MSYS2)
    )
)

:: 5. Копируем папку с плагинами GStreamer
echo.
echo Копирование плагинов GStreamer...
if not exist "%GSTREAMER_LIB_PATH%" (
    echo Ошибка: Папка плагинов GStreamer не найдена по пути: %GSTREAMER_LIB_PATH%
    goto :eof
)
xcopy /s /e /y "%GSTREAMER_LIB_PATH%" "%GSTREAMER_RELEASE_PLUGINS_PATH%\" >nul
echo - Скопированы плагины GStreamer в %GSTREAMER_RELEASE_PLUGINS_PATH%

echo.
echo === Копирование зависимостей завершено! ===
echo Ваша программа готова к распространению в папке: %RELEASE_PATH%
echo.
echo Проверьте содержимое папки и запустите %APP_EXE_NAME% из нее.
pause
endlocal