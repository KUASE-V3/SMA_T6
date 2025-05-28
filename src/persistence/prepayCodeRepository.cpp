#include "persistence/prepayCodeRepository.h"
#include <map> // ���� ����ҷ� std::map ���

namespace persistence {


// �����ڵ�� ������ ���� ��ȸ
domain::PrePaymentCode PrepayCodeRepository::findByCode(const std::string& code) {
    auto it = codes_.find(code);
    if (it != codes_.end()) {
        return it->second;
    }
    // �ڵ带 ã�� ���� ���, �⺻ ������ PrePaymentCode ��ü�� ��ȯ�մϴ�.
    // UC14 (�����ڵ� ��ȿ�� ����)�� ���� ��Ȳ E1: "�ý��ۿ� ����� ���� ���� �����ڵ��̸�, �ý����� �����޽����� ȣ���� �����ش�." [cite: 30]
    // �� �������丮 �޼ҵ�� ã�� ���Ҹ� �ϸ�, ���� ���� �޽��� ǥ�ô� ���� �����̳� ��Ʈ�ѷ����� ó���մϴ�.
    return domain::PrePaymentCode(); // �⺻ ��ü ��ȯ
}

// ������ ���� ����
void PrepayCodeRepository::save(const domain::PrePaymentCode& prepayCode) {
    // prepayCode�� code_attribute�� Ű�� ����Ͽ� ������ �����ϰų� ������Ʈ�մϴ�.
    // PrePaymentCode ��ü ��ü�� �ڵ� ���ڿ��� ������ �����Ƿ�, prepayCode.getCode()�� ����մϴ�.
    codes_[prepayCode.getCode()] = prepayCode;
    // UC12 (�����ڵ� �߱�) �ܰ迡�� ������ �����ڵ尡 ����˴ϴ�. [cite: 25]
    // UC15 (��������û ���� �� ��� Ȯ��) �ܰ迡���� �ٸ� ���Ǳ��� ��û���� �ڵ尡 ����� �� �ֽ��ϴ�. [cite: 32]
    // �̶� PrePayment.code = AuthCode.code�� �����ϴ� ������ ����. [cite: 32]
}

// ������ �ڵ� ���� ���� (��: ACTIVE -> USED)
bool PrepayCodeRepository::updateStatus(const std::string& code, domain::CodeStatus newStatus) {
    auto it = codes_.find(code);
    if (it != codes_.end()) {

        if (newStatus == domain::CodeStatus::USED) {
            if (it->second.isUsable()) { // ACTIVE ������ ���� USED�� ���� ����
                it->second.markAsUsed(); // UC14: "��ġ�� ��� AuthCode�� �����Ų��." [cite: 30] (���Ḧ USED ���·� �ؼ�)
                return true;
            } else {
                // �̹� USED �����̰ų� �ٸ� ��Ȱ�� ������ �� ����
                return false; 
            }
        }
        return false; // �� �� ���� ������ ������
    }
    return false; // �ڵ带 ã�� ����
}

} // namespace persistence