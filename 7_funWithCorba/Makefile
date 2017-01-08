SRC=BankingApp.idl AccountServant.java BankServant.java InterbankServant.java

all: classes

classes: $(SRC)
	idlj -fall -td . -verbose BankingApp.idl
	javac *.java BankingApp/*.java

startInterbank:
	orbd -ORBInitialPort 1050 &
	sleep 1
	java com.sun.corba.se.impl.activation.ServerTool -ORBInitialPort 1050 -cmd register -server InterbankServer -applicationName interbank -classpath .

stopInterbank:
	java com.sun.corba.se.impl.activation.ServerTool -ORBInitialPort 1050 -cmd shutdown -applicationName interbank

startBank:
	java BankServer -ORBInitialPort 1050

startClient:
	java Client -ORBInitialPort 1050

.PHONY: clean reset

clean:
	rm -f *.class BankingApp/* &

reset: clean
	pkill orbd &
	rm -rf orb.db &
	rm -f *.dat &
