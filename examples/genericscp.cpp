#include <boost/asio.hpp>

#include "dcmtkpp/Association.h"
#include "dcmtkpp/EchoSCP.h"
#include "dcmtkpp/FindSCP.h"
#include "dcmtkpp/StoreSCP.h"
#include "dcmtkpp/registry.h"
#include "dcmtkpp/SCPDispatcher.h"
#include "dcmtkpp/SCP.h"

#include "dcmtkpp/message/CEchoRequest.h"
#include "dcmtkpp/message/CFindRequest.h"
#include "dcmtkpp/message/CFindResponse.h"
#include "dcmtkpp/message/CStoreRequest.h"

class FindGenerator: public dcmtkpp::SCP::ResponseGenerator
{
public:
    FindGenerator()
    : _request(nullptr), _state(State::NotInitialized)
    {
        // Nothing to do
    }

    virtual ~FindGenerator()
    {
        // Nothing to do.
    }

    virtual void initialize(dcmtkpp::message::Request const & request)
    {
        this->_request =
            std::make_shared<dcmtkpp::message::CFindRequest>(request);
        this->_state = State::Pending;

        dcmtkpp::DataSet data_set_1;
        data_set_1.add(dcmtkpp::registry::PatientName, {"Hello^World"});
        data_set_1.add(dcmtkpp::registry::PatientID, {"1234"});
        this->_responses.push_back(data_set_1);

        dcmtkpp::DataSet data_set_2;
        data_set_2.add(dcmtkpp::registry::PatientName, {"Doe^John"});
        data_set_2.add(dcmtkpp::registry::PatientID, {"5678"});
        this->_responses.push_back(data_set_2);

        this->_response_iterator = this->_responses.begin();
    }

    virtual bool done() const
    {
        return this->_state == State::Done;
    }

    virtual dcmtkpp::message::Response get() const
    {
        if(this->_state == State::NotInitialized)
        {
            throw dcmtkpp::Exception("Find generator not initialized");
        }
        else if(this->_state == State::Pending)
        {
            return dcmtkpp::message::CFindResponse(
                this->_request->get_message_id(),
                dcmtkpp::message::CFindResponse::Pending,
                *this->_response_iterator);
        }
        else if(this->_state == State::Final)
        {
            return dcmtkpp::message::CFindResponse(
                this->_request->get_message_id(),
                dcmtkpp::message::CFindResponse::Success);
        }
        else if(this->_state == State::Done)
        {
            throw dcmtkpp::Exception("Generator is finished");
        }
        else
        {
            throw dcmtkpp::Exception("Unknown state");
        }
    }

    virtual void next()
    {
        if(this->_state == State::NotInitialized)
        {
            throw dcmtkpp::Exception("Find generator not initialized");
        }
        else if(this->_state == State::Pending)
        {
            ++this->_response_iterator;
            if(this->_response_iterator == this->_responses.end())
            {
                this->_state = State::Final;
            }
        }
        else if(this->_state == State::Final)
        {
            this->_state = State::Done;
        }
        else if(this->_state == State::Done)
        {
            throw dcmtkpp::Exception("Generator is finished");
        }
        else
        {
            throw dcmtkpp::Exception("Unknown state");
        }
    }


private:
    enum class State
    {
        NotInitialized,
        Pending,
        Final,
        Done
    };

    std::shared_ptr<dcmtkpp::message::CFindRequest> _request;
    State _state;

    std::vector<dcmtkpp::DataSet> _responses;
    std::vector<dcmtkpp::DataSet>::const_iterator _response_iterator;
};

dcmtkpp::Value::Integer echo(dcmtkpp::message::CEchoRequest const & request)
{
    std::cout << "Received echo\n";
    std::cout << "  ID: " << request.get_message_id() << "\n";
    std::cout << "  Affected SOP Class UID: " << request.get_affected_sop_class_uid() << "\n";
    return dcmtkpp::message::Response::Success;
}

dcmtkpp::Value::Integer store(dcmtkpp::message::CStoreRequest const & request)
{
    auto const patient_name =
        request.get_data_set().as_string(dcmtkpp::registry::PatientName)[0];
    std::cout << "Storing " << patient_name << "\n";
    return dcmtkpp::message::Response::Success;
}

int main()
{
    dcmtkpp::Association association;
    association.receive_association(boost::asio::ip::tcp::v4(), 11112);

    std::cout
        << "Received association from "
        << association.get_peer_host() << ":" << association.get_peer_port()
        << "\n";

    auto const & contexts =
        association.get_negotiated_parameters().get_presentation_contexts();
    std::cout << "Presentation contexts (" << contexts.size() << ")\n";
    for(auto const & context: contexts)
    {
        std::cout
            << "    "
            << dcmtkpp::registry::uids_dictionary.at(context.abstract_syntax).name
            << ": "
            << dcmtkpp::registry::uids_dictionary.at(context.transfer_syntaxes[0]).name
            << ", "
            << (context.scu_role_support?"SCU":"")
            << ((context.scu_role_support & context.scp_role_support)?"/":"")
            << (context.scp_role_support?"SCP":"")
            << std::endl;
    }

    auto echo_scp = std::make_shared<dcmtkpp::EchoSCP>(association, echo);
    auto find_scp = std::make_shared<dcmtkpp::FindSCP>(
        association, std::make_shared<FindGenerator>());
    auto store_scp = std::make_shared<dcmtkpp::StoreSCP>(association, store);

    dcmtkpp::SCPDispatcher dispatcher(association);
    dispatcher.set_scp(dcmtkpp::message::Message::Command::C_ECHO_RQ, echo_scp);
    dispatcher.set_scp(dcmtkpp::message::Message::Command::C_FIND_RQ, find_scp);
    dispatcher.set_scp(
        dcmtkpp::message::Message::Command::C_STORE_RQ, store_scp);

    bool done = false;
    while(!done)
    {
        try
        {
            dispatcher.dispatch();
        }
        catch(dcmtkpp::AssociationReleased const &)
        {
            std::cout << "Peer released association" << std::endl;
            done = true;
        }
        catch(dcmtkpp::AssociationAborted const & e)
        {
            std::cout
                << "Peer aborted association, "
                << "source: " << int(e.source) << ", "
                << "reason: " << int(e.reason)
                << std::endl;
            done = true;
        }
    }
}
