if [ $# -ne 1 ] # 获取命令行的参数个数，如果个数不等于1则进入then
then
        echo "Usage: usr_monitor username" # 参数不等于1就报错
	sleep 1
        exit
fi
user_input=$1 # 将第一个参数赋给user_input
echo -e "You will monitor [$user_input]\n"

username=`who |awk '{print $1}'` # 通过who获取登录的用户信息，将信息传给awk命令，通过awk命令获得第一列的值（用户名）
echo -e "Current user list is:\n$username\n"
 
find=$(echo $username | grep "${user_input}") #获取username中的用户名信息，传递给grep命令，通过grep获取与user_input相同的用户名，最后将其赋值给find（未找到返回空）
 
while [ "$find" == "" ] #若未在登录用户中找到命令行输入的用户名则进入循环
do
        echo "waiting user [$user_input] ..."
        sleep 5		#等待5秒
        username=`who |awk '{print $1}'` #更新登录用户的信息
        find=$(echo $username | grep "${user_input}") #检查是否找到，找到则退出循环
done
echo "[$user_input] is log on"
