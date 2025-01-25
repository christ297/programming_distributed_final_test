

import java.io.IOException;
import java.net.*;
import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

public class ServicesImpl extends UnicastRemoteObject implements Services
{
    private static final ExecutorService threadPool = Executors.newFixedThreadPool(2);
    private InetSocketAddress address;
    private MulticastSocket socket;
    private byte[] data;

    public ServicesImpl(byte[] data, String multicast, int port) throws RemoteException
    {
        this.data = data;

        try
        {
            socket = new MulticastSocket();
            socket.setBroadcast(true);
            socket.setReuseAddress(true);
            address = new InetSocketAddress(multicast, port);
            socket.joinGroup(address, NetworkInterface.getByInetAddress(InetAddress.getByName(multicast)));
        }
        catch (IOException e)
        {
            System.err.println(e.getMessage());
        }
    }


    @Override
    public synchronized MessageTemperature getMessageTemperature() throws ExecutionException, InterruptedException
    {

        Future<MessageTemperature> temperatureFuture = threadPool.submit(() -> {

            DatagramPacket packet = new DatagramPacket(data, data.length, address.getAddress(), address.getPort());

            socket.receive(packet);

            System.out.println("[MESSAGE - Systeme Central [Serveur GestionConsole]] : addresse = "
                    + packet.getAddress() + ", port = " + packet.getPort());

            return MessageTemperature.fromBytes(packet.getData(), packet.getLength());
        });

        return temperatureFuture.get();
    }

    public synchronized void centralCall() throws IOException
    {
        String etat = " [Serveur RMI] : OK";

        DatagramPacket datagramPacket = new DatagramPacket(etat.getBytes(), etat.length(), address.getAddress(), address.getPort());

        socket.send(datagramPacket);

        System.out.println("[MESSAGE - Systeme Central [Serveur RMI]] : addresse = "
                + datagramPacket.getAddress() + ", port = " + datagramPacket.getPort());
    }

    public void setData(byte[] data) {
        this.data = data;
    }
}
