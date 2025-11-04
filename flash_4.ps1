# docker cp zmk-dev:/workspaces/zmk/app/build/zephyr/zmk.uf2 F:/
Push-Location ../zmk4_1/app
west flash -r uf2
Pop-Location