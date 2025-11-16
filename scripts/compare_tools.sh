
PID_ALVO=$1
#Armazena o PID alvo passado como argumento, caso não tenha argumento ele mostra o menu

if [ -z "$PID_ALVO" ]; then
    echo "Uso: ./compare_tools.sh <PID>"
    exit 1
fi

#Organiza as saidas
echo "---------------------------------------------"
echo " Comparação de métricas para o PID: $PID_ALVO"
echo "---------------------------------------------"

echo ""
#Mostra as metricas coletadas
echo "===> Métricas coletadas pelo resource-monitor"
./resource-monitor -r "$PID_ALVO"
echo ""

#Exibe métrica apenas do PID desejado por tabela
echo "===> Métricas coletadas no ps aux:"
ps -p "$PID_ALVO" -o pid,pcpu,pmem,vsz,rss
echo ""

#mostra memoria total, livre e usada pelo sistema
echo "===> Memória total do sistema (free -m)"
free -m
echo ""

#panorama geral dos 10 primeiros processos
echo "===> Uso de CPU (top -b -n 1)"
top -b -n 1 | head -n 10
echo ""

#linha final
echo "---------------------------------------------"
echo " Comparação finalizada"
echo "---------------------------------------------"