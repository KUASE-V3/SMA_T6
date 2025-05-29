#pragma once

#include <string>
#include <vector>
#include <functional>
#include <unordered_map> // std::map에서 변경됨

// --- 필요한 타입들의 전체 정의를 먼저 포함 ---
#include "network/message.hpp"         // network::Message 및 network::Message::Type 정의
#include "network/MessageSender.hpp"   // network::MessageSender 정의
#include "network/MessageReceiver.hpp" // network::MessageReceiver 및 network::EnumClassHash 정의
#include "service/ErrorService.hpp"    // service::ErrorService 정의

namespace service {

// 콜백 함수 타입 정의: 수신된 네트워크 메시지를 처리하기 위한 일반 핸들러
using GenericMessageHandler = std::function<void(const network::Message& message)>;

/**
 * @brief 다른 자판기와의 메시지 송수신을 담당하는 서비스입니다.
 * MessageSender와 MessageReceiver를 사용하여 네트워크 통신을 수행하고,
 * 수신된 메시지에 따라 등록된 핸들러(주로 UserProcessController의 메소드)를 호출합니다.
 */
class MessageService {
public:
    /**
     * @brief MessageService 생성자.
     * @param sender 메시지 송신을 위한 MessageSender 객체에 대한 참조.
     * @param receiver 메시지 수신을 위한 MessageReceiver 객체에 대한 참조.
     * @param errorService 오류 처리를 위한 ErrorService 객체에 대한 참조.
     * @param myVmId 현재 자판기의 고유 ID.
     * @param myCoordX 현재 자판기의 X 좌표 (RESP_STOCK 메시지 구성 시 사용).
     * @param myCoordY 현재 자판기의 Y 좌표 (RESP_STOCK 메시지 구성 시 사용).
     */
    MessageService(
        network::MessageSender& sender,
        network::MessageReceiver& receiver,
        service::ErrorService& errorService,
        const std::string& myVmId,
        int myCoordX,
        int myCoordY
    );

    /**
     * @brief MessageReceiver를 통해 메시지 수신을 시작하도록 설정합니다.
     * 내부적으로 각 메시지 타입에 대해 onMessageReceived를 호출하는 핸들러를 등록하고,
     * MessageReceiver의 start()를 호출합니다.
     */
    void startReceivingMessages();

    // --- 메시지 송신 메소드 ---

    /**
     * @brief 재고 조회 요청 (REQ_STOCK)을 모든 다른 자판기에 브로드캐스트합니다. (UC8)
     * PFR 표1: 재고 확인 요청 시의 msg format 준수.
     * @param drinkCode 조회할 음료의 코드.
     */
    void sendStockRequestBroadcast(const std::string& drinkCode);

    /**
     * @brief 특정 자판기에 선결제 재고 확보 요청 (REQ_PREPAY)을 전송합니다. (UC16)
     * PFR 표3: 선결제 요청 시의 msg format 준수.
     * @param targetVmId 대상 자판기의 ID.
     * @param drinkCode 예약할 음료의 코드.
     * @param authCode 생성된 인증 코드.
     */
    void sendPrepaymentReservationRequest(const std::string& targetVmId, const std::string& drinkCode, const std::string& authCode);

    /**
     * @brief 다른 자판기의 재고 조회 요청(REQ_STOCK)에 대한 응답(RESP_STOCK)을 전송합니다. (UC17)
     * PFR 표2: 재고 확인 응답 시의 msg format 준수.
     * @param destinationVmId 응답을 받을 자판기(요청을 보낸 자판기)의 ID.
     * @param drinkCode 조회 요청받은 음료의 코드.
     * @param currentStock 해당 음료의 현재 재고량 (0~99).
     */
    void sendStockResponse(const std::string& destinationVmId, const std::string& drinkCode, int currentStock);

    /**
     * @brief 다른 자판기의 선결제 재고 확보 요청(REQ_PREPAY)에 대한 응답(RESP_PREPAY)을 전송합니다. (UC15)
     * PFR 표4: 선결제 가능 여부 응답 시의 msg format 준수.
     * @param destinationVmId 응답을 받을 자판기(요청을 보낸 자판기)의 ID.
     * @param drinkCode 요청받은 음료의 코드.
     * @param reservedItemNum 실제로 확보(예약)된 음료의 수량 (성공 시 요청 수량, 실패 시 0).
     * @param available 재고 확보 및 선결제 처리 가능 여부 (T/F).
     */
    void sendPrepaymentReservationResponse(const std::string& destinationVmId, const std::string& drinkCode, int reservedItemNum, bool available);

    // --- 메시지 수신 핸들러 등록 ---

    /**
     * @brief 특정 메시지 타입에 대한 처리 핸들러를 등록합니다.
     * UserProcessController 등 외부에서 이 메소드를 호출하여 각 메시지 타입에 대한 콜백 함수를 설정합니다.
     * @param type 처리할 메시지의 타입 (network::Message::Type).
     * @param handler 해당 타입의 메시지를 받았을 때 호출될 함수 객체.
     */
    void registerMessageHandler(network::Message::Type type, GenericMessageHandler handler);

private:
    network::MessageSender& messageSender_;     // 메시지 송신용 객체
    network::MessageReceiver& messageReceiver_; // 메시지 수신용 객체
    service::ErrorService& errorService_;       // 오류 처리용 객체

    std::string myVmId_;    // 이 자판기의 ID
    int myCoordX_;          // 이 자판기의 X 좌표
    int myCoordY_;          // 이 자판기의 Y 좌표

    /**
     * @brief MessageReceiver로부터 모든 타입의 메시지를 받아,
     * 등록된 핸들러 맵(messageHandlers_)을 참조하여 적절한 핸들러를 호출하는 내부 콜백.
     * @param msg 수신된 network::Message 객체.
     */
    void onMessageReceived(const network::Message& msg);

    // 등록된 메시지 타입별 핸들러들을 저장하는 맵
    // Key: network::Message::Type, Value: GenericMessageHandler, Hash: network::EnumClassHash
    std::unordered_map<network::Message::Type, GenericMessageHandler, network::EnumClassHash> messageHandlers_;
};

} // namespace service
