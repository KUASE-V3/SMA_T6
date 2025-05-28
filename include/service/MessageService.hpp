#pragma once

#include <string>
#include <map> // ErrorInfo�� contextDetails �뵵 (������)
// #include <chrono> // timestamp�� ���� ����

namespace service {

// �߻� ������ ��ü���� ���� ������ (���������� ������Ʈ)
enum class ErrorType {
    // User Input/Interaction Errors
    INVALID_MENU_CHOICE,
    INVALID_DRINK_CODE_FORMAT, // ���� ���� (��: ���ڰ� �ƴ� ��)
    USER_RESPONSE_TIMEOUT,

    // Inventory/Stock Errors
    DRINK_NOT_FOUND,        // �Է��� ���� �ڵ尡 ��ü 20���� ����
    DRINK_OUT_OF_STOCK,     // ���� ���Ǳ� ��� ����
    INSUFFICIENT_STOCK_FOR_DECREASE, // ��� ���� �õ� �� ���� ��� ���� (���� ���� ���ɼ�)

    // Payment Errors
    PAYMENT_PROCESSING_FAILED,
    PAYMENT_TIMEOUT,
    PAYMENT_SYSTEM_UNAVAILABLE, // �ܺ� ���� �ý��� ���� ����

    // Prepayment/AuthCode Errors
    AUTH_CODE_INVALID_FORMAT,   // �Էµ� ���� �ڵ� ���� ����
    AUTH_CODE_NOT_FOUND,        // �������� �ʴ� ���� �ڵ�
    AUTH_CODE_ALREADY_USED,     // �̹� ���� ���� �ڵ�
    AUTH_CODE_GENERATION_FAILED,// ���� �ڵ� ���� ���� (���� ����)
    STOCK_RESERVATION_FAILED_AT_OTHER_VM, // �ٸ� ���Ǳ� ������ ��� Ȯ�� ���� ����

    // Network/Message Errors
    NETWORK_COMMUNICATION_ERROR,// �Ϲ� ��� ����
    MESSAGE_SEND_FAILED,
    MESSAGE_RECEIVE_FAILED,
    INVALID_MESSAGE_FORMAT,     // ���� �޽��� �Ľ�/���� ����
    RESPONSE_TIMEOUT_FROM_OTHER_VM, // �ٸ� ���Ǳ� ���� �ð� �ʰ�

    // System/Internal Errors
    REPOSITORY_ACCESS_ERROR,    // �������丮 ���� �� ���� (���� I/O ��)
    UNEXPECTED_SYSTEM_ERROR,    // ����ġ ���� ���� �ý��� ����
    INITIALIZATION_FAILED       // �ý��� �ʱ�ȭ ����
};

// ���� ó�� �� UserProcessController�� ���ؾ� �� �ൿ ����
enum class ErrorResolutionLevel {
    RETRY_INPUT,              // ����ڿ��� �Է� ��õ� ��û (���� ���� ����)
    RETURN_TO_PREVIOUS_STEP,  // ���� ���� �ܰ�� ���ư���
    RETURN_TO_MAIN_MENU,      // ���� �޴�(���� ��� ǥ��)�� ���ư���
    NOTIFY_AND_STAY,          // ����ڿ��� �˸��� ���� ���� ���� (��: �ܼ� ���� �޽���)
    SYSTEM_FATAL_ERROR        // ���� �Ұ���, �ý��� �ߴ� �Ǵ� ���� ��� ����
};

// ErrorService�� ���޵� ���� ���� �� ó�� ���
struct ErrorInfo {
    ErrorType type;
    std::string userFriendlyMessage; // ����ڿ��� ������ �޽���
    ErrorResolutionLevel resolutionLevel;
    // std::map<std::string, std::string> details; // (������) ���� �� ����

    ErrorInfo(ErrorType t, std::string msg, ErrorResolutionLevel level)
        : type(t), userFriendlyMessage(std::move(msg)), resolutionLevel(level) {}
};

class ErrorService {
public:
    ErrorService() = default;
    // ~ErrorService() = default; // ��� ��� �� ��

    /**
     * @brief �߻��� ���� Ÿ�԰� �߰� ������ �������� ErrorInfo ��ü�� �����Ͽ� ��ȯ�մϴ�.
     * UserProcessController�� �� ErrorInfo�� �޾� �ļ� ��ġ�� �����մϴ�.
     * @param occurredErrorType �߻��� ��ü���� ������ Ÿ��.
     * @param additionalContext (������) ���� �޽����� ���Ե� �߰����� ���� ���� (��: ���� �̸�).
     * @return ������ ErrorInfo ��ü.
     */
    ErrorInfo processOccurredError(ErrorType occurredErrorType, const std::string& additionalContext = "");

private:
    // ErrorType�� ���� �⺻ userFriendlyMessage�� ErrorResolutionLevel�� �����ϴ� ���� ����
    std::string getDefaultUserMessageForError(ErrorType type, const std::string& context);
    ErrorResolutionLevel getDefaultResolutionLevelForError(ErrorType type);
};

} // namespace service