#include "stdafx.h"
#include <algorithm>
#include <boost/format.hpp>
#include "NetworkIO.h"
#include "MatrixIO.h"
#include "Connection.h"

namespace Net
{
	template<typename T>
	bool CompareUniquePtr(const std::unique_ptr<T>& ptr, const T& ref)
	{
		return ptr.get() == &ref;
	}


	template<typename T> std::string classname(const T& obj)
	{
		std::string fullName = std::string(typeid(obj).name());
		assert(fullName.substr(0, 6) == "class ");
		return fullName.substr(6, std::string::npos);
	}

	template<typename T> std::string classname_ptr(const T* obj)
	{
		if (!obj)
			return "null";
		else
			return classname(*obj);
	}

	typedef unsigned version;
	const version layer_version = 1;
	const version connection_version = 1;
	const version network_version = 1;
	const char separator = ' ';

	std::ostream& operator<< (std::ostream& stream, const Layer& layer)
	{
		stream << layer_version << separator;
		stream << layer.units << " ";
		stream << classname_ptr(layer.function.get()) << std::endl;
		stream << layer.bias.transpose() << std::endl;
		return stream;
	}

	std::unique_ptr<Function> FunctionFactory(const std::string& typeName)
	{
		if (typeName == "null")
			return nullptr;
		if (typeName == "Net::Boolean")
			return std::make_unique<Boolean>();
		else if (typeName == "Net::Linear")
			return std::make_unique<Linear>();
		else if (typeName == "Net::Sigmoid")
			return std::make_unique<Sigmoid>();
		else if (typeName == "Net::Stochastic")
			return std::make_unique<Stochastic>();
		else
			throw std::domain_error((boost::format("Syntax error, unknown activation function = '%s'.") % typeName).str());
	}

	std::istream& operator>> (std::istream& stream, Layer& layer)
	{
		version v;
		stream >> v;
		if (v >= 1)
		{
			std::string functionName;
			stream >> layer.units >> functionName;
			layer.function = std::move(FunctionFactory(functionName));
			layer.bias = Eigen::VectorXd(layer.Size());
			stream >> layer.bias;
		}

		return stream;
	}

	namespace Connection
	{
		std::ostream& operator<< (std::ostream& stream, const Base& connection)
		{
			stream << connection_version << std::endl;
			stream << connection.weights << std::endl;

			return stream;
		}


		std::istream& operator>> (std::istream& stream, Base& connection)
		{
			version v;
			stream >> v;
			if (v >= 1)
			{
				connection.weights.resize(connection.a.Size(), connection.b.Size());
				stream >> connection.weights;
			}

			return stream;
		}
	}


	std::ostream& operator<< (std::ostream& stream, const Network& network)
	{
		stream << network_version << separator;
		stream << network.layers.size() << separator << network.connections.size() << std::endl;
		for (auto layerIt = network.layers.begin(); layerIt != network.layers.end(); ++layerIt)
		{
			const Layer& layer = *layerIt->get();
			stream << layerIt - network.layers.begin() << separator << classname(layer) << separator;
			stream << layer;
		}
		for (auto connectionIt = network.connections.begin(); connectionIt != network.connections.end(); ++connectionIt)
		{
			const Connection::Base& connection = *connectionIt->get();
			unsigned a = std::find_if(network.layers.begin(), network.layers.end(), [connection](const std::unique_ptr<Layer>& ptr){ return CompareUniquePtr(ptr, connection.A()); }) - network.layers.begin();
			unsigned b = std::find_if(network.layers.begin(), network.layers.end(), [connection](const std::unique_ptr<Layer>& ptr){ return CompareUniquePtr(ptr, connection.B()); }) - network.layers.begin();
			stream << connectionIt - network.connections.begin() << separator << classname(connection) << separator << a << separator << b << separator;
			stream << connection;
		}
		return stream;
	}


	std::istream& operator>> (std::istream& stream, Network& network)
	{
		version v;
		stream >> v;
		if (v >= 1)
		{
			size_t layerCount, connectionCount;
			stream >> layerCount >> connectionCount;
			for (size_t layerIt = 0; layerIt < layerCount; ++layerIt)
			{
				unsigned layerIndex;
				std::string layerType;
				stream >> layerIndex >> layerType;
				// todo: layer factory
				if (layerType == "Net::InputLayer")
					network.layers.emplace_back(std::make_unique<InputLayer>());
				else if (layerType == "Net::HiddenLayer")
					network.layers.emplace_back(std::make_unique<HiddenLayer>());
				else
					throw std::domain_error((boost::format("Syntax error,  layer[%d].type = '%s' unknown.") % layerIt % layerType).str());
				Layer& layer = *network.layers.back().get();
				stream >> layer;
			}
			for (size_t connectionIt = 0; connectionIt < connectionCount; ++connectionIt)
			{
				unsigned connectionIndex, aIndex, bIndex;
				std::string connectionType;
				stream >> connectionIndex >> connectionType >> aIndex >> bIndex;
				if (aIndex >= layerCount)
					throw std::out_of_range((boost::format("Connection [%d].A = %d out of range.") % connectionIndex % aIndex).str());

				if (bIndex >= layerCount)
					throw std::out_of_range((boost::format("Connection [%d].B = %d out of range.") % connectionIndex % bIndex).str());

				if (connectionType == "Net::Connection::Directed")
					network.connections.emplace_back(std::make_unique<Connection::Directed>(*network.layers[aIndex].get(), *network.layers[bIndex].get()));
				else if (connectionType == "Net::Connection::Undirected")
					network.connections.emplace_back(std::make_unique<Connection::Undirected>(*network.layers[aIndex].get(), *network.layers[bIndex].get()));
				else
					throw std::domain_error((boost::format("Syntax error,  connection[%d].Type = '%s' unknown.") % connectionIndex % connectionType).str());
				Connection::Base& connection = *network.connections.back().get();
				stream >> connection;
			}
		}
		return stream;
	}
}
