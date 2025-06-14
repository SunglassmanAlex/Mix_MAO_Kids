#!/bin/bash

echo "=== 2048游戏项目GitHub更新脚本 ==="
echo

# 检查当前目录
echo "当前目录: $(pwd)"
echo

# 检查Git状态
echo "检查Git状态..."
git status
echo

# 检查远程仓库
echo "当前配置的远程仓库:"
git remote -v
echo

# 检查分支信息
echo "分支信息:"
git branch -a
echo

# 检查最近的提交
echo "最近的提交:"
git log --oneline -5
echo

# 询问用户要推送到哪个远程仓库
echo "请选择要推送的远程仓库:"
echo "1) origin (my2048_FDU_oop)"
echo "2) Mix_MAO_Kids"
echo "3) 推送到所有远程仓库"
echo

read -p "请输入选择 (1/2/3): " choice

case $choice in
    1)
        echo "正在推送到 origin..."
        git push origin master
        ;;
    2)
        echo "正在推送到 Mix_MAO_Kids..."
        git push Mix_MAO_Kids master
        ;;
    3)
        echo "正在推送到所有远程仓库..."
        git push origin master
        git push Mix_MAO_Kids master
        ;;
    *)
        echo "无效选择，退出。"
        exit 1
        ;;
esac

echo
echo "更新完成！"
echo "您可以访问以下链接查看项目:"
echo "- https://github.com/SunglassmanAlex/my2048_FDU_oop"
echo "- https://github.com/SunglassmanAlex/Mix_MAO_Kids" 