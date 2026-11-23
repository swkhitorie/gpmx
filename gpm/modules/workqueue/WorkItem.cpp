#include "WorkItem.hpp"

namespace px4
{

namespace wq_configurations
{
const wq_config_t &rate_ctrl            = ::wq_config_rate_ctrl;

const wq_config_t &spi0                 = ::wq_config_spi0;
const wq_config_t &spi1                 = ::wq_config_spi1;
const wq_config_t &spi2                 = ::wq_config_spi2;
const wq_config_t &spi3                 = ::wq_config_spi3;
const wq_config_t &spi4                 = ::wq_config_spi4;
const wq_config_t &spi5                 = ::wq_config_spi5;
const wq_config_t &spi6                 = ::wq_config_spi6;

const wq_config_t &i2c0                 = ::wq_config_i2c0;
const wq_config_t &i2c1                 = ::wq_config_i2c1;
const wq_config_t &i2c2                 = ::wq_config_i2c2;
const wq_config_t &i2c3                 = ::wq_config_i2c3;
const wq_config_t &i2c4                 = ::wq_config_i2c4;

const wq_config_t &nav_and_controllers  = ::wq_config_nav_and_controllers;

const wq_config_t &ins0                 = ::wq_config_ins0;
const wq_config_t &ins1                 = ::wq_config_ins1;
const wq_config_t &ins2                 = ::wq_config_ins2;
const wq_config_t &ins3                 = ::wq_config_ins3;

const wq_config_t &hp_default           = ::wq_config_hp_default;
const wq_config_t &uavcan               = ::wq_config_uavcan;

const wq_config_t &uart0                = ::wq_config_uart0;
const wq_config_t &uart1                = ::wq_config_uart1;
const wq_config_t &uart2                = ::wq_config_uart2;
const wq_config_t &uart3                = ::wq_config_uart3;
const wq_config_t &uart4                = ::wq_config_uart4;
const wq_config_t &uart5                = ::wq_config_uart5;
const wq_config_t &uart6                = ::wq_config_uart6;
const wq_config_t &uart7                = ::wq_config_uart7;
const wq_config_t &uart8                = ::wq_config_uart8;
const wq_config_t &ttyacm0              = ::wq_config_ttyacm0;
const wq_config_t &uart_unknown         = ::wq_config_uart_unknown;

const wq_config_t &lp_default           = ::wq_config_lp_default;

const wq_config_t &test1                = ::wq_config_test1;
const wq_config_t &test2                = ::wq_config_test2;
} // namespace wq_configurations

const px4::wq_config_t &ins_instance_to_wq(uint8_t instance)
{
	switch (instance) {
	case 0: return wq_configurations::ins0;

	case 1: return wq_configurations::ins1;

	case 2: return wq_configurations::ins2;

	case 3: return wq_configurations::ins3;
	}

	return wq_configurations::ins0;
}

const px4::wq_config_t &serial_port_to_wq(const char *serial)
{
	if (serial == NULL) {
		return wq_configurations::uart_unknown;

	} else if (strstr(serial, "ttyS0")) {
		return wq_configurations::uart0;

	} else if (strstr(serial, "ttyS1")) {
		return wq_configurations::uart1;

	} else if (strstr(serial, "ttyS2")) {
		return wq_configurations::uart2;

	} else if (strstr(serial, "ttyS3")) {
		return wq_configurations::uart3;

	} else if (strstr(serial, "ttyS4")) {
		return wq_configurations::uart4;

	} else if (strstr(serial, "ttyS5")) {
		return wq_configurations::uart5;

	} else if (strstr(serial, "ttyS6")) {
		return wq_configurations::uart6;

	} else if (strstr(serial, "ttyS7")) {
		return wq_configurations::uart7;

	} else if (strstr(serial, "ttyS8")) {
		return wq_configurations::uart8;

	}

	return wq_configurations::uart_unknown;
}

const px4::wq_config_t &device_bus_to_wq(const char *bus)
{
	if (bus == NULL) {
		return wq_configurations::hp_default;

	} else if (strstr(bus, "i2cdev0")) {
		return wq_configurations::i2c0;

	} else if (strstr(bus, "i2cdev1")) {
		return wq_configurations::i2c1;

	} else if (strstr(bus, "i2cdev2")) {
		return wq_configurations::i2c2;

	} else if (strstr(bus, "i2cdev3")) {
		return wq_configurations::i2c3;

	} else if (strstr(bus, "i2cdev4")) {
		return wq_configurations::i2c4;

	} else if (strstr(bus, "spidev0")) {
		return wq_configurations::spi0;

	} else if (strstr(bus, "spidev1")) {
		return wq_configurations::spi1;

	} else if (strstr(bus, "spidev2")) {
		return wq_configurations::spi2;

	} else if (strstr(bus, "spidev3")) {
		return wq_configurations::spi3;

	} else if (strstr(bus, "spidev4")) {
		return wq_configurations::spi4;

	} else if (strstr(bus, "spidev5")) {
		return wq_configurations::spi5;

	} else if (strstr(bus, "spidev6")) {
		return wq_configurations::spi6;

	}

	return wq_configurations::hp_default;
}

WorkItem::WorkItem(const char *name, const wq_config_t &config)
{
	workitem_config_entry(&_item, &WorkItem::RunTrampoline, this);

	if (!workitem_init(&_item, name, &config)) {
		KMERROR("init failed");
	}
}

WorkItem::WorkItem(const char *name, const WorkItem &work_item)
{
	workitem_config_entry(&_item, &WorkItem::RunTrampoline, this);

	intrusive_node_init(&_item.sorted_list_node);
	intrusive_node_init(&_item.queue_node);

	workitem_init_method2(&_item, name, &work_item._item);
}

WorkItem::~WorkItem()
{
	Deinit();
}

void WorkItem::RunTrampoline(void *arg)
{
	static_cast<WorkItem *>(arg)->Run();
}

void WorkItem::ScheduleClear()
{
	workitem_schedule_clear(&_item);
}

bool WorkItem::Init(const wq_config_t &config)
{
	return workitem_change_queue(&_item, &config);
}

void WorkItem::Deinit()
{
	workitem_deinit(&_item);
}

float WorkItem::elapsed_time() const
{
	return workitem_elapsed_time(raw());
}

float WorkItem::average_rate() const
{
	return workitem_average_rate(raw());
}

float WorkItem::average_interval() const
{
	return workitem_average_interval(raw());
}

void WorkItem::print_run_status()
{
	workitem_printstatus(raw());

	_item._run_count = 0;
}

} // namespace px4
