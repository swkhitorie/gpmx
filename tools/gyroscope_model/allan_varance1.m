function [sigma, tau, Err] = allan_varance1(y0, tau0)
% Calculate Allan variance.
%
% Prototype: [sigma, tau, Err] = avar(y0, tau0)
% Inputs: y - data (gyro in deg/hur; acc in g)
%         tau0 - sampling interval
% Outputs: sigma - Allan variance
%          tau - Allan variance correlated time
%          Err - Allan variance error boundary

data = load('gyro_sample_3h.txt');
y0 = data(:,1:3);
tau0 = 0.01;
    N = length(y0);
    y = y0; NL = N;
    for k = 1:log2(N)
        sigma(k,1) = sqrt(1/(2*(NL-1))*sum((y(2:NL)-y(1:NL-1)).^2)); % diff&std
        tau(k,1) = 2^(k-1)*tau0;      % correlated time 相关时间
        Err(k,1) = 1/sqrt(2*(NL-1));  % error boundary
        NL = floor(NL/2);             %floor(X)四舍五入X
        if NL<3
            break;
        end
        y = 1/2*(y(1:2:2*NL) + y(2:2:2*NL));  % mean & half data length
    end
    figure;
    subplot(211)
    plot(tau0*(1:N)', y0); grid
    subplot(212)
    loglog(tau, sigma, '-+', tau, [sigma.*(1+Err),sigma.*(1-Err)], 'r--'); grid
    
    p0 = [1 1 1 1 1];
    p = lsqcurvefit(@gyro_model1,p0,tau(k,1),sigma(k,1));
    q = sqrt(abs(p));
    R = q(1)*sqrt(2);%速率斜坡系数
    K = q(2)*sqrt(3);%速率随机游走系数
    B = q(3)*(2/3);%零偏稳定性
    N = q(4);%角度随机游走系数
    Q = q(5)/sqrt(3);%量化噪声系数
    xishu = [R K B N Q]'
