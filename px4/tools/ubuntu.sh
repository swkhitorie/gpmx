
echo
echo "Installing PX4 general dependencies"

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

sudo apt-get install \
	build-essential \
	cmake \
	cppcheck \
	g++ \
	gcc \
	gdb \
	git \
	make \
	python3 \
	python3-dev \
	python3-pip \
	python3-setuptools \
	python3-wheel \
	shellcheck \
	;

echo "Installing PX4 Python3 dependencies"
python3 -m pip install --user -r ${DIR}/uorb_gen/tools/requirements.txt
