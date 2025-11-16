PID_ALVO=$1

if [ -z "$PID_ALVO" ]; then
    echo "Uso: ./compare_tools.sh <PID>"
    exit 1
fi

echo "---------------------------------------------"
echo " Comparação de métricas para o PID: $PID_ALVO"
echo "---------------------------------------------"

echo ""
echo "===> Métricas coletadas pelo resource-monitor"
./resource-monitor -r "$PID_ALVO"
echo ""

echo "===> Métricas coletadas no ps aux:"
ps -p "$PID_ALVO" -o pid,pcpu,pmem,vsz,rss
echo ""

echo "===> Memória total do sistema (free -m)"
free -m
echo ""

echo "===> Uso de CPU (top -b -n 1)"
top -b -n 1 | head -n 10
echo ""

echo "---------------------------------------------"
echo " Comparação finalizada"
echo "---------------------------------------------"