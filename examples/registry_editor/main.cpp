#include <ctime>
#include <iostream>
#include <string>
#include <any>

#include "CLI11.hpp"

#include "asio_cyphalpp.hpp"
#include <cyphalppheartbeatpublish.hpp>

#include <cyphalppbinaryregistry_impl.hpp>
#include <cyphalppboostjsonregistry_impl.hpp>

#include <uavcan/node/GetInfo_1_0.hpp>
#include <uavcan/node/ExecuteCommand_1_0.hpp>

using namespace std::literals::chrono_literals;
using namespace boost::asio;
using boost::asio::ip::address_v4;
using boost::asio::ip::udp;
using GetInfo = uavcan::node::GetInfo::Service_1_0;
using ExecuteCommand = uavcan::node::ExecuteCommand::Service_1_0;


using RegistryPtr = std::shared_ptr<cyphalpp::registry::Registry>;
using ImplPtr = std::shared_ptr<cyphalpp::registry::RegistryImpl>;
using ImplFactory = std::function<ImplPtr()>;

struct RegistryFactory{
    std::vector<std::any> values;
    std::function<void(CLI::App*, std::vector<std::any>&)> options;
    std::function<ImplFactory(const std::vector<std::any>&)> factory;
};

struct OpenRegistry{
    std::string registryType;
    ImplPtr registry;
};

struct SetValue{
    ImplPtr registry;
    size_t valueType;
    cyphalpp::registry::Name name;
    cyphalpp::registry::Value value;
};

using NamedRegistries = std::map<std::string, std::shared_ptr<RegistryFactory>>;
using ConversionResult = tl::expected<boost::json::value, std::string>;

template<typename T>
ConversionResult convertStringTo(const std::string& type, const std::string& val){
    T v;
    if(not CLI::detail::lexical_cast<T>(val, v)){
        return tl::unexpected("Wrong value: " + val + " for type ");
    }
    return boost::json::value{
        {"type", "s"},
        {"value", boost::json::array{ v } },
    };
}

using ValueConversions = std::unordered_map<
    size_t,
    std::function<ConversionResult (const std::string&) > >;

int main(int argc, char** argv){

    static ValueConversions valueConversions({
        {
            cyphalpp::registry::Value::VariantType::IndexOf::string,
            [](const std::string& val){
                return boost::json::value{
                    {"type", cyphalpp::registry::Value::VariantType::IndexOf::string},
                    {"value", val},
                };
            }
        },
    });

    static NamedRegistries registryTypes({
        {"binary", std::make_shared<RegistryFactory>(RegistryFactory{
            std::vector<std::any>({std::make_any<std::string>()}),
            [](CLI::App* a, std::vector<std::any>& values){
                a->description("BinaryRegistry"); 
                a->add_option_function<std::string>(
                    "filename", 
                    [&values](const std::string& s){values[0] = s; }
                )->required();
            },
            [](const std::vector<std::any>& values)->ImplFactory{
                auto filename = std::any_cast<std::string>(values.at(0));
                return [filename](){
                    return std::make_unique<cyphalpp::registry::BinaryRegistry>(std::move(filename));
                };
            }
        })},
        {"json", std::make_shared<RegistryFactory>(RegistryFactory{
            std::vector<std::any>({std::make_any<std::string>()}),
            [](CLI::App* a, std::vector<std::any>& values){
                a->description("Json registry"); 
                a->add_option_function<std::string>(
                    "filename", 
                    [&values](const std::string& s){values[0] = s; }
                )->required();
            },
            [](const std::vector<std::any>& values)->ImplFactory{
                auto filename = std::any_cast<std::string>(values.at(0));
                return [filename](){
                    return std::make_unique<cyphalpp::registry::BoostJsonRegistry>(std::move(filename));
                };
            }
        })},
    });
    
    std::unordered_map<std::string, ImplPtr> registries;
    CLI::App app{"Registry editor"};

    OpenRegistry openReg{};
    std::string regName{};
    auto openApp = app.add_subcommand("open", "opens a registry")->immediate_callback();
    openApp->callback([&regName, &openReg, &openApp, &registries]{
        auto sc = openApp->get_subcommands()[0];
        auto& regType = registryTypes.at(sc->get_name());
        auto reg = regType->factory(regType->values)();
        reg->load();
        registries.insert({regName, std::move(reg)});
    });
    openApp->add_option("name", regName, "alias for this registry")->required();
    for(auto reg: registryTypes){
        auto a = openApp->add_subcommand(reg.first);
        reg.second->options(a, reg.second->values);
    }
    openApp->require_subcommand(1);

    SetValue setValue{};
    auto setApp = app.add_subcommand("set", "sets a new value")->immediate_callback();
    setApp->callback([&setValue](){
        setValue.registry->set(setValue.name, setValue.value);
    });
    std::unordered_map<std::string_view, size_t> stringToTypes = cyphalpp::registry::stringToType();
    constexpr static size_t VALUE_TYPE_JSON = cyphalpp::registry::Value::VariantType::MAX_INDEX + 1;
    stringToTypes.emplace("json", VALUE_TYPE_JSON);
    auto value_type = setApp->add_option("-t,--value-type", setValue.valueType)
        ->default_str("string")
        ->transform(CLI::IsMember{stringToTypes});
    setApp->add_option("registry", "Registry in which the value should be set")
        ->check([&setValue, &registries](const std::string& val)->std::string{
            auto r = registries.find(val);
            if(r == registries.end()){
                return "Registry " + val + " isn't open yet!";
            }
            setValue.registry = r->second;
            return {};
        })
        ->required();
    setApp->add_option("name", "Register to set value to")
        ->check([&setValue](const std::string& val) -> std::string{
            setValue.name = cyphalpp::registry::types::N(val);
            return {};
        })
        ->required();
    setApp->add_option("value", "New value for the register")
        ->check([&setValue](const std::string& val) -> std::string{
            boost::json::value parsed{};
            // if(setValue.valueType == VALUE_TYPE_JSON){
            boost::json::error_code ec;
            parsed = boost::json::parse(val, ec);
            if(ec){
                return std::string("Error parsing value: ") + ec.message();
            }
            auto parsedValue = cyphalpp::registry::fromJValue(parsed);
            if(parsedValue.is_empty()){
                return "Wrong value!";
            }
            setValue.value = parsedValue;
            return {};
        })
        ->required();
    
    auto listApp = app.add_subcommand("list", "shows all values")->immediate_callback();
    listApp->callback([&registries](){
        for(auto r: registries){
            std::cout << "Registry: " << r.first << '\n';

            auto Nr = r.second->size();
            for(uint16_t i = 0; i< Nr; ++i){
                auto n = r.second->getName(i).name;
                if(n.name.empty()) continue;
                auto val = r.second->get(n);
                std::cout << cyphalpp::registry::name_view(n) << '\t' << cyphalpp::registry::toJValue(val.value) << '\n';
            }
            std::cout << std::endl;
        }
    });
    
    auto commitApp = app.add_subcommand("commit", "commits all registries")->immediate_callback();
    commitApp->callback([&registries](){
        for(auto r: registries){
            r.second->save();
        }
    });

    app.require_subcommand();

    CLI11_PARSE(app, argc, argv);

    return 0;
    int retcode = 0;
    try{
        io_service service{};


        auto stop = [&service, &retcode](int code = 1){
            service.stop();
            retcode = code;
        };
        // Construct a signal set registered for process termination.
        boost::asio::signal_set signals(service, SIGINT, SIGTERM);

        // Start an asynchronous wait for one of the signals to occur.
        signals.async_wait([&stop](const boost::system::error_code& error,int signal_number){
            if (!error)
            {
                stop();
            }
        });

        auto udpSockets = cyphalpp::asio::asioUdpSocket(service);
        auto timers = cyphalpp::asio::asioTimer(service);
        
        cyphalpp::registry::Registry reg{std::make_unique<cyphalpp::registry::BinaryRegistry>(argv[1])};
        using namespace cyphalpp::registry::types;
        using cyphalpp::registry::Value;
        auto iface = reg.setDefault(N("uavcan.udp.iface"), S("127.0.0.1")).as_value<std::string>();
        auto node_id = reg.setDefault(N("uavcan.node.id"), U16(10)).as_value<uint16_t>();
        std::cerr << "Starting on " << iface << " nodeid:" << node_id << "\n";

        cyphalpp::CyphalUdp cy((address_v4::from_string(iface).to_uint() & static_cast<uint32_t>(~0xFFFFU)) | static_cast<uint32_t>(node_id), udpSockets, timers);
        cyphalpp::HeartbeatPublish<> hb(cy, timers());

        service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}