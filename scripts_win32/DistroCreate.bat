cd ..

robocopy app/ distro/ *.dll
robocopy app/ distro/ Intrinsic.exe
robocopy app/ distro/ Intrinsic.pdb
robocopy app/ distro/ IntrinsicEd.exe
robocopy app/ distro/ IntrinsicEd.pdb
robocopy app/ distro/ settings.json
robocopy app/config/ distro/config/ material_pass_config.json
robocopy app/config/ distro/config/ renderer_config.json
robocopy app/config/ distro/config/ renderer_config_simple.json
robocopy app/platforms/ distro/platforms /E
robocopy app/managers/ distro/managers /E
robocopy app/media/ distro/media /E
robocopy app/scripts distro/scripts /E
robocopy app/tools distro/tools /E
robocopy app/worlds distro/worlds /E

cd scripts_win32

timeout 2
