

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;


public class UdpEchoMultiThreadServer {
	final static int MAXBUFFER = 512;
	public static void main(String[] args){
		if(args.length < 1)
		{
			System.out.println("ªÁøÎπ˝ : java -jar UdpEchoMultiThreadServer.jar [port]");
			System.exit(0);
		}
		int arg_port = Integer.parseInt(args[0]);
		new UdpEchoMultiThreadServer().work(arg_port);
	}
	void work(int arg_port){
		int port = arg_port;
		
		String receiveString;
		try{
			DatagramSocket Dsocket = new DatagramSocket(port); /*fill in the blank*/
			System.out.println("Running the UDP Echo Server");
			System.out.println("IP address is "+InetAddress.getLocalHost().getHostAddress()+", Port number is "+port);
			while(true){
				/* UDP packet
				 * udp server socket
				 * udp server socket
				 */
				byte[] rece_buffer = new byte[MAXBUFFER];
				DatagramPacket recv_packet = new DatagramPacket(rece_buffer, MAXBUFFER) ; /*fill in the blank*/
				Dsocket.receive(recv_packet);
				receiveString = new String(recv_packet.getData()).substring(0, recv_packet.getLength());
				
				System.out.println("Server received data:"+receiveString);
				new EchoSender(Dsocket, recv_packet, receiveString).start();
			}

		}catch(IOException e){
			System.out.println(e);
		}
	}
	class EchoSender extends Thread{
		DatagramSocket Dsocket;
		DatagramPacket echo_packet;
		String object;
		public EchoSender(DatagramSocket socket, DatagramPacket packet, String object) {
			// TODO Auto-generated constructor stub
			Dsocket = socket;
			echo_packet = packet;
			this.object = object;
		}
		public void run() {
			// TODO Auto-generated method stub
			
			DatagramPacket send_packet = new DatagramPacket(object.getBytes(),object.getBytes().length,echo_packet.getAddress(),echo_packet.getPort());
			try {
				Dsocket.send(send_packet);
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			super.run();
		}
	}

}
