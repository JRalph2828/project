package UDP_Chatting_LLC;

public class Signaling {

	public static boolean ACKNOTIFY = false; // check for receive ACK
	public static boolean TIMENOTIFY = false; // check for Timeout
	public static boolean SABMENOTIFY = false; // check for SABME
	public static boolean SABMEUANOTIFY = false;
	public static boolean DISCNOTIFY = false; // check for DISC
	public static boolean DISCUANOTIFY = false;

	public synchronized void Timeoutnotifying() { // Timeout acquired
		TIMENOTIFY = false;
		notify();
	}

	public synchronized void initACK() { // initialize ACKNOTIFY
		ACKNOTIFY = false;
	}

	public synchronized void ACKnotifying() { // receive ACK
		ACKNOTIFY = true;
		notify();
	}

	public synchronized void waitingACK() { // wait for ACK
		try {
			initACK();
			wait();
		} catch (InterruptedException e) {
			System.out.println("InterruptedException : " + e);
		}
	}

	public synchronized void initSABMEUA() { // initialize SABMEUANOTIFY 
		SABMEUANOTIFY = false;
	}

	public synchronized void SABMEUAnotifying() { // receive SABMEUA
		SABMEUANOTIFY = true;
		notify();
	}

	public synchronized void waitingSABMEUA() { // waiting for SABMEUA
		try {
			initSABMEUA();
			wait();
		} catch (InterruptedException e) {
			System.out.println("InterruptedException : " + e);
		}
	}

	public synchronized void initSABME() { // initialize SABMENOTIFY 
		SABMENOTIFY = false;
	}

	public synchronized void SABMEnotifying() { // receive SABME
		SABMENOTIFY = true;
		notify();
	}

	public synchronized void waitingSABME() { // waiting for SABME
		try {
			initSABME();
			wait();
		} catch (InterruptedException e) {
			System.out.println("InterruptedException : " + e);
		}
	}

	public synchronized void initDISC() { // initialize DISC 
		DISCNOTIFY = false;
	}

	public synchronized void DISCnotifying() { // receive DISC
		DISCNOTIFY = true;
		notify();
	}

	public synchronized void waitingDISC() { // waiting for DISC
		try {
			initDISC();
			wait();
		} catch (InterruptedException e) {
			System.out.println("InterruptedException : " + e);
		}
	}
	
	public synchronized void initDISCUA() { // initialize DISCUA
		DISCUANOTIFY = false;
	}

	public synchronized void DISCUAnotifying() { // receive DISCUA
		DISCUANOTIFY = true;
		notify();
	}

	public synchronized void waitingDISCUA() { // waiting for DISCUA
		try {
			initDISCUA();
			wait();
		} catch (InterruptedException e) {
			System.out.println("InterruptedException : " + e);
		}
	}
}
