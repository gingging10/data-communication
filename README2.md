#packet.h의 기능
데이터, ACK, EOT 패킷을 정의하고 패킷 이벤트를 로그로 기록

#sender.c
파일을 패킷으로 분할하여 수신자에게 전송하는 역할을 하며 
    packet.type = EOT;
    packet.seq_num = seq_num;
    packet.ack_num = 0;
    packet.length = 0;를 통해 EOT를 보낸다.
    EOT 전송은 데이터 전송을 완료한 후에 보내게 되는 것이다.
    패킷으로 변환한 파일을 UDP를 사용하여 보냄 처리한다.

  #receiver.c
  데이터 패킷 수신과 ACK 패킷을 보내는 역할을 하는 함수
  
