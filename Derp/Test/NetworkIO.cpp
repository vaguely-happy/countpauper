#include "stdafx.h"
#include <boost/test/unit_test.hpp>
#include <strstream>
#include "../Net/NetworkIO.h"
#include <Eigen/Dense>

BOOST_AUTO_TEST_SUITE(NetworkIO);

BOOST_AUTO_TEST_CASE(WriteEmpty)
{
	Network net;
	net.Reset();

	std::stringstream stream;
	stream << net;
	BOOST_REQUIRE(!stream.bad());
	BOOST_CHECK_EQUAL(stream.str(), "1 0 0\n");
}

BOOST_AUTO_TEST_CASE(ReadEmpty)
{
	Network net;
	std::stringstream stream("1 0 0\n");
	stream >> net;
	BOOST_REQUIRE(!stream.bad());
	BOOST_REQUIRE(!stream.eof());
	BOOST_CHECK_EQUAL(net.GetLayers().size(), 0);
}

BOOST_AUTO_TEST_CASE(WriteLayer)
{
	Network net;
	net.Add(3);
	net.Reset();

	std::stringstream stream;

	stream << net;
	BOOST_REQUIRE(!stream.bad());
	BOOST_CHECK_EQUAL(stream.str(), "1 1 0\n0 InputLayer 1 3\n0 0 0\n");
}

BOOST_AUTO_TEST_CASE(ReadLayer)
{
	Network net;
	std::stringstream stream("1 1 0\n0 InputLayer 1 2\n0.1 -0.2\n");
	stream >> net;
	BOOST_REQUIRE(!stream.bad());
	BOOST_REQUIRE(!stream.eof());
	BOOST_REQUIRE_EQUAL(net.GetLayers().size(), 1);
	BOOST_REQUIRE_EQUAL(net.GetLayers().front()->Size(), 2);
	Eigen::Vector2d refBias;
	refBias << 0.1, -0.2;
	BOOST_CHECK_EQUAL(net.GetLayers().front()->Bias(), refBias);
}

BOOST_AUTO_TEST_CASE(WriteConnection)
{
	Network net;
	net.Add(net.Add(3), 2);
	net.Reset();

	std::stringstream stream;
	stream << net;
	BOOST_REQUIRE(!stream.bad());
	BOOST_CHECK_EQUAL(stream.str(), "1 2 1\n0 InputLayer 1 3\n0 0 0\n1 HiddenLayer 1 2\n0 0\n0 Connection 0 1 1\n0 0\n0 0\n0 0\n");
}

BOOST_AUTO_TEST_CASE(ReadConnection)
{
	Network net;
	std::stringstream stream("1 2 1\n0 InputLayer 1 2\n0.2 -0.3\n1 HiddenLayer 1 2\n-0.5 0.9\n0 Connection 0 1 1\n0 0\n0 0\n");
	stream >> net;
	BOOST_REQUIRE(!stream.bad());
	BOOST_REQUIRE(!stream.eof());
	BOOST_CHECK_EQUAL(net.GetLayers().size(), 2);
	Eigen::Vector2d refBias;
	refBias << 0.2, -0.3;
	BOOST_CHECK_EQUAL(net.GetLayers()[0]->Bias(), refBias);
	refBias << -0.5, 0.9;
	BOOST_CHECK_EQUAL(net.GetLayers()[1]->Bias(), refBias);
	const auto& weights = net.GetLayers()[0]->GetConnections()[0]->Weights();
	BOOST_CHECK_EQUAL(weights, Eigen::Matrix2d().Zero());
}

BOOST_AUTO_TEST_CASE(ConnectionIndexException)
{
	Network net;
	std::stringstream stream1("1 1 1\n0 InputLayer 1 0\n\n0 Connection 1 0 1\n\n");
	BOOST_CHECK_THROW(stream1 >> net, std::out_of_range);
	std::stringstream stream2("1 1 1\n0 InputLayer 1 0\n\n0 Connection 0 1 1\n\n");
	BOOST_CHECK_THROW(stream2 >> net, std::out_of_range);
}

BOOST_AUTO_TEST_CASE(LayerTypeException)
{
	Network net;
	std::stringstream stream("1 1 0\n0 Bogus 1 0\n\n");
	BOOST_CHECK_THROW(stream >> net, std::domain_error);
}

BOOST_AUTO_TEST_CASE(ConnectionTypeException)
{
	Network net;
	std::stringstream stream("1 1 1\n0 InputLayer 1 0\n\n0 Bogus 0 0 0\n\n");
	BOOST_CHECK_THROW(stream >> net, std::domain_error);
}
BOOST_AUTO_TEST_SUITE_END()