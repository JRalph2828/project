package UDP_Chatting;

public class Signaling {
	
	public static boolean ACKNOTIFY = false;	// boolean variable for check if ACK is received
	public static boolean TIMENOTIFY = false;	// boolean variable for check Timeout
	
	// if Timeout occur
	public synchronized void Timeoutnotify() {
		TIMENOTIFY = false;
		notify();
	}

	// reset ACKNOTIFY
	public synchronized void initACK() {
		ACKNOTIFY = false;
	}

	// if ACK is received
	public synchronized void ACKnotify() {
		ACKNOTIFY = true;
		notify();
	}
	

	// waiting for ACK
	public synchronized void waitingACK() {
		try {
			initACK();
			wait();
		} catch (InterruptedException e) {
			System.out.println("InterruptedException : " + e);
		}
	}
}
