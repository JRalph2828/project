package UDP_Chatting;

import java.util.Timer;
import java.util.TimerTask;

public class Timeout {
	
	Timer timer = new Timer();								// create Timer
	TimeoutTask[] TimerArr = new TimeoutTask[16];		// create TimeoutTask
	Signaling signal;

	boolean temp = false;

	// set Timeout 
	public void TimeoutSet(int index , int ms , Signaling signal ) {
		this.signal = signal;
		this.TimerArr[index] = new TimeoutTask(index);
		timer.schedule(TimerArr[index], ms);
	}
	
	// reset Timeout 
	public void TimeoutReset(int index) {
		int i = index;
		if (temp)
			System.out.println("Timer reset!(no." + i + ")");
		TimerArr[i].cancel();
	}

	// class TimeoutTask
	class TimeoutTask extends TimerTask {
		int num;

		TimeoutTask(int num) {
			this.num = num;
		}

		public void run() {
			if (temp)
				System.out.println("Time's up! ");
			signal.Timeoutnotify();
			this.cancel();
		}
	}
}
